// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "sdc.h"

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#define SPI_RW  0x20003000
#define SPI_CTR 0x20003004

#define SPI_SS 0x1
#define SPI_SCK 0x2

#define SD_START_TOKEN  0xFE

#define SD_MAX_READ_ATTEMPTS    300000
#define SD_MAX_WRITE_ATTEMPTS   750000

#define CS_ENABLE()  *(volatile uint32_t *)SPI_CTR = 0
#define CS_DISABLE() *(volatile uint32_t *)SPI_CTR = SPI_SS

int is_hardware(void);

static uint8_t spi_transfer(uint8_t mosi)
{
    *(volatile uint32_t *)SPI_RW = (uint32_t)mosi;
    return *(volatile uint32_t *)SPI_RW;
}

static void sdc_command(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t m[5];
    m[0] = cmd | 0x40;
    m[1] = (uint8_t)(arg >> 24);
    m[2] = (uint8_t)(arg >> 16);
    m[3] = (uint8_t)(arg >> 8);
    m[4] = (uint8_t)(arg);

    for (int i = 0; i < 5; ++i)
        spi_transfer(m[i]);

    // transmit crc
    spi_transfer(crc | 0x1);
}

static uint8_t sdc_read_res1(void)
{
    uint8_t i = 0, res1 = 0xFF;

    // keep polling until actual data received
    while((res1 = spi_transfer(0xFF)) == 0xFF)
    {
        i++;

        // if no data is received for 8 bytes, break
        if (i > 8)
            break;
    }

    return res1;
}

static void sdc_read_res3_7(uint8_t *res)
{
    // read response 1 in R7
    res[0] = sdc_read_res1();

    // if error reading R1, return
    if (res[0] > 1) return;

    // read remaining bytes
    res[1] = spi_transfer(0xFF);
    res[2] = spi_transfer(0xFF);
    res[3] = spi_transfer(0xFF);
    res[4] = spi_transfer(0xFF);
}

static void sdc_power_up_seq(void)
{
    // make sure the card is deselected
    CS_DISABLE();

    // give sd card time to power up
    usleep(1000);

    // send 80 clock cycles to synchronize
    for (uint8_t i = 0; i < 10; ++i)
        spi_transfer(0xFF);

    // deselect SD card
    CS_DISABLE();
    spi_transfer(0xFF);
}

static uint8_t sdc_go_idle_state(void)
{
    // assert chip select
    spi_transfer(0xFF);
    CS_ENABLE();
    spi_transfer(0xFF);

    // send CMD0
    sdc_command(0, 0x00000000, 0x94);

    // read response
    uint8_t res1 = sdc_read_res1();

    // deassert chip select
    spi_transfer(0xFF);
    CS_DISABLE();
    spi_transfer(0xFF);

    return res1;
}

static void sdc_send_if_cond(uint8_t *res)
{
    // assert chip select
    spi_transfer(0xFF);
    CS_ENABLE();
    spi_transfer(0xFF);

    // send cmd8
    sdc_command(8, 0x000001AA, 0x86);

    // read response
    sdc_read_res3_7(res);

    // deassert chip select
    spi_transfer(0xFF);
    CS_DISABLE();
    spi_transfer(0xFF);
}

static void sdc_read_ocr(uint8_t *res)
{
    // assert chip select
    spi_transfer(0xFF);
    CS_ENABLE();
    spi_transfer(0xFF);

    // send cmd58
    sdc_command(58, 0x00000000, 0x00);

    // read response
    sdc_read_res3_7(res);

    // deassert chip select
    spi_transfer(0xFF);
    CS_DISABLE();
    spi_transfer(0xFF);
}

static uint8_t sdc_send_app(void)
{
    // assert chip select
    spi_transfer(0xFF);
    CS_ENABLE();
    spi_transfer(0xFF);

    // send cmd55
    sdc_command(55, 0x00000000, 0x00);

    // read response
    uint8_t res1 = sdc_read_res1();

    // deassert chip select
    spi_transfer(0xFF);
    CS_DISABLE();
    spi_transfer(0xFF);

    return res1;
}

static uint8_t sdc_send_op_cond()
{
    // assert chip select
    spi_transfer(0xFF);
    CS_ENABLE();
    spi_transfer(0xFF);

    // send cmd41
    sdc_command(41, 0x40000000, 0x00);

    // read response
    uint8_t res1 = sdc_read_res1();

    // deassert chip select
    spi_transfer(0xFF);
    CS_DISABLE();
    spi_transfer(0xFF);

    return res1;
}

static bool sdc_init_internal(void)
{
    uint8_t res[5];

    // start the power up sequence
    sdc_power_up_seq();

    // command card to go idle (CMD0)
    res[0] = sdc_go_idle_state();

    // send if conditions (CMD8)
    sdc_send_if_cond(res);

    // read operation conditions register (CMD58)
    sdc_read_ocr(res);

    // send operating condition (ACMD41) until no longer in idle
    for (int i = 0; i < 100; ++i) {
        // send app (CMD55)
        sdc_send_app();
        res[0] = sdc_send_op_cond();
        if (res[0] == 0x00 || res[0] != 0x01)
            break;
    }

    return res[0] == 0x00;
}


bool sdc_init(void) {
    if (is_hardware()) {
        int retries = 0;
        while (retries++ < 3) {
            if (sdc_init_internal())
                return true;
            usleep(100000);
        }
        return false;
    }
    return true;
}

void sdc_dispose(void) {
}

static bool sdc_read_single_block_hw(uint32_t addr, uint8_t *buf)
{
    uint8_t token;
    uint8_t res1, read;
    unsigned int read_attempts;

    // set token to none
    token = 0xFF;

    // assert chip select
    spi_transfer(0xFF);
    CS_ENABLE();
    spi_transfer(0xFF);

    // send CMD17
    sdc_command(17, addr, 0x00);

    // read R1
    res1 = sdc_read_res1();

    // if response received from the card
    if (res1 != 0xFF) {
        // wait for a response token (timeout == 100ms)
        read_attempts = 0;
        while (++read_attempts != SD_MAX_READ_ATTEMPTS)
            if ((read = spi_transfer(0xFF)) != 0xFF)
                break;

            // if response token is 0xFE
            if (read == 0xFE) {
                // read block
                for (uint16_t i = 0; i < SDC_BLOCK_LEN; ++i)
                    *buf++ = spi_transfer(0xFF);

                // read 16-bit CRC
                spi_transfer(0xFF);
                spi_transfer(0xFF);
            }

            // set token to card response
            token = read;
    }

    // deassert chip select
    spi_transfer(0xFF);
    CS_DISABLE();
    spi_transfer(0xFF);

    return res1 == 0x00 && token == 0xFE;
}

static bool sdc_write_single_block_hw(uint32_t addr, const uint8_t *buf)
{
    uint8_t token;
    uint8_t res1, write;
    unsigned int write_attempts;

    // set token to none
    token = 0xFF;

    // assert chip select
    spi_transfer(0xFF);
    CS_ENABLE();
    spi_transfer(0xFF);

    // send CMD24
    sdc_command(24, addr, 0x00);

    // read R1
    res1 = sdc_read_res1();

    if (res1 == 0x00) {
        // send start token
        spi_transfer(SD_START_TOKEN);

        // write buffer to card
        for (uint16_t i = 0; i < SDC_BLOCK_LEN; ++i)
            spi_transfer(buf[i]);
    }

    // if response received from the card
    if (res1 != 0xFF) {
        // wait for a response token (timeout == 250ms)
        write_attempts = 0;
        while (++write_attempts != SD_MAX_WRITE_ATTEMPTS)
            if ((write = spi_transfer(0xFF)) != 0xFF) {
                token = 0xFF;
                break;
            }

        // if data accepted
        if ((write & 0x1F) == 0x05) {
            // set token to data accepted
            token = 0x05;

            // wait for write to finish (timeout == 250ms)
            write_attempts = 0;
            while (spi_transfer(0xFF) == 0x00)
                if (++write_attempts == SD_MAX_WRITE_ATTEMPTS) {
                    token = 0x00;
                    break;
                }
        }
    }

    // deassert chip select
    spi_transfer(0xFF);
    CS_DISABLE();
    spi_transfer(0xFF);

    return res1 == 0x00 && token == 0x05;    
}

bool sdc_read_single_block(uint32_t addr, uint8_t* buf) {
    if (is_hardware()) {
        return sdc_read_single_block_hw(addr, buf);
    } else {
        int i;
        *(uint32_t *)(0x2000000C) = addr * SDC_BLOCK_LEN;
        for (size_t i = 0; i < SDC_BLOCK_LEN; i++) {
            *buf = *(uint32_t *)(0x20000010);
            buf++;
        }

        return true;
    }
}

bool sdc_write_single_block(uint32_t addr, const uint8_t* buf) {
    if (is_hardware()) {
        return sdc_write_single_block_hw(addr, buf);
    } else {
        *(uint32_t *)(0x2000000C) = addr * SDC_BLOCK_LEN;
        for (size_t i = 0; i < SDC_BLOCK_LEN; i++) {
            *(uint32_t *)(0x20000010) = *buf;
            buf++;
        }

        return true;
    }
}
