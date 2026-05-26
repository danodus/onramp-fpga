// Copyright (c) 2025-2026 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <memory>
#include <chrono>
#include <deque>
#include <fstream>
#include <sstream>

#include <fcntl.h>
#include <termios.h>

#include <verilated.h>
#include <iostream>
#include <filesystem>

// Include model header, generated from Verilating "top.v"
#include "Vtop.h"

#define SDRAM_MEM_SIZE (32*1024*1024/2)

double sc_time_stamp()
{
    return 0.0;
}

static bool raw_mode = false;
static struct termios orig_termios;

void disable_raw_mode(int fd) {
    if (raw_mode) {
        tcsetattr(fd,TCSAFLUSH,&orig_termios);
        raw_mode = false;
        printf("Raw mode disabled.\n");
    }
}

void exit_handler(void) {
    disable_raw_mode(STDIN_FILENO);
}

int enable_raw_mode(int fd) {
    struct termios raw;
    int flags;

    if (raw_mode) return 0;
    if (!isatty(STDIN_FILENO)) goto fatal;
    atexit(exit_handler);
    if (tcgetattr(fd,&orig_termios) == -1) goto fatal;

    raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    // Put terminal in raw mode after flushing
    if (tcsetattr(fd,TCSAFLUSH,&raw) < 0) goto fatal;

    // Set to non-blocking mode
    flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    raw_mode = true;
    return 0;

fatal:
    errno = ENOTTY;
    return -1;
}


int main(int argc, char **argv, char **env)
{

    uint32_t sdc_addr = 0;
    // Read sd.img
    std::ifstream input("../sd.img", std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        printf("Unable to open ../sd.img\n");
        return 1;
    }

    // Get the file size (C++17)
    size_t file_size = std::filesystem::file_size("../sd.img");
    // Calculate the number of uint32_t elements
    size_t numElements = file_size / sizeof(uint32_t);

    std::vector<uint32_t> sdc_data(numElements);

    // Read all elements into the vector's underlying array
    input.read(reinterpret_cast<char*>(sdc_data.data()), file_size);

    if (input.gcount() == file_size) {
        std::cout << "Successfully read " << sdc_data.size() << " elements." << std::endl;
    } else {
        std::cerr << "Error reading all data." << std::endl;
    }

    input.close();

    enable_raw_mode(STDIN_FILENO);

    // Create logs/ directory in case we have traces to put under it
    Verilated::mkdir("logs");

    // Construct a VerilatedContext to hold simulation time, etc.
    // Multiple modules (made later below with Vtop) may share the same
    // context to share time, or modules may have different contexts if
    // they should be independent from each other.

    // Using unique_ptr is similar to
    // "VerilatedContext* contextp = new VerilatedContext" then deleting at end.
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};

    // Set debug level, 0 is off, 9 is highest presently used
    // May be overridden by commandArgs argument parsing
    contextp->debug(0);

    // Randomization reset policy
    // May be overridden by commandArgs argument parsing
    contextp->randReset(0);

    // Verilator must compute traced signals
    contextp->traceEverOn(true);

    // Pass arguments so Verilated code can see them, e.g. $value$plusargs
    // This needs to be called before you create any model
    contextp->commandArgs(argc, argv);

    // Construct the Verilated model, from Vtop.h generated from Verilating "top.v".
    // Using unique_ptr is similar to "Vtop* top = new Vtop" then deleting at end.
    // "TOP" will be the hierarchical name of the module.
    const std::unique_ptr<Vtop> top{new Vtop{contextp.get(), "TOP"}};

    std::deque<uint8_t> keys;

    uint16_t *sdram_mem = new uint16_t[SDRAM_MEM_SIZE];
    uint32_t sdram_rows[4] = {0, 0, 0, 0};  // 2^13 = 8192 rows per bank
    uint32_t sdram_col = 0; // 2^9 = 512 columns
    uint32_t sdram_addr = 0;
    uint8_t burst_counter = 0;

    int delay_burst = 0;
    bool write_sdram = false;
    bool read_sdram = false;    

    for (size_t i = 0; i < SDRAM_MEM_SIZE; ++i)
        sdram_mem[i] = 0x0000;

    // Set Vtop's input signals
    top->i_rst = 1;

    bool done = false;
    int exit_value = 0;
    while (!contextp->gotFinish() && !done)
    {
        int nread;
        char c;
        nread = read(STDIN_FILENO, &c, 1);
        if (nread > 0) {
            if (c == 3) {
                // CTRL-C - ETX
                done = true;
            } else {
                keys.emplace_front(c);
            }
        }

        top->clk = !top->clk;

        // if negedge clk sdram
        if (!top->clk) {

            if (!top->sdram_cs_n_o) {
                // activate
                uint32_t sdram_bank = top->sdram_ba_o;
                if (!top->sdram_ras_n_o && top->sdram_cas_n_o && top->sdram_we_n_o) {
                    sdram_rows[sdram_bank] = top->sdram_a_o;
                    //printf("ACT bank=%d, row=%d\r\n", sdram_bank, sdram_rows[sdram_bank]);
                }
                uint32_t sdram_row = sdram_rows[sdram_bank];
                if (top->sdram_ras_n_o && !top->sdram_cas_n_o) {
                    // read or write
                    sdram_col = top->sdram_a_o & 0x1FF;
                    sdram_addr = 8192 * 512 * sdram_bank + 512 * sdram_row + sdram_col;
                    assert(sdram_addr < 8192 * 512 * 4);
                    if (!top->sdram_we_n_o) {
                        // Write
                        //printf("WRITE bank=%d, row=%d, col=%d (addr=0x%x), mask=%d\r\n", sdram_bank, sdram_row, sdram_col, sdram_addr*2, ~top->sdram_dqm_o & 0x03);
                        burst_counter = 0;
                        delay_burst = 0;
                        write_sdram = true;
                    } else {
                        // Read
                        //printf("READ bank=%d, row=%d, col=%d (addr=0x%x)\r\n", sdram_bank, sdram_row, sdram_col, sdram_addr*2);
                        burst_counter = 0;
                        delay_burst = 3;
                        read_sdram = true;
                    }
                }

                if (top->sdram_ras_n_o && top->sdram_cas_n_o && !top->sdram_we_n_o) {
                    // end of burst
                    //printf("EOB\n");
                    write_sdram = false;
                    //read_sdram = false;
                }
            }

            uint32_t addr = sdram_addr + burst_counter;
            assert(addr < 8192 * 512 * 4);
            uint8_t mask = ~top->sdram_dqm_o & 0x03;

            if (write_sdram) {
                //printf("Write %x at addr %x (%d), mask=%x\r\n", top->sdram_dq_io, addr*2, burst_counter, mask);
                switch (mask) {
                    case 0:
                        break;
                    case 1:
                        sdram_mem[addr] = (sdram_mem[addr] & 0xFF00) | (top->sdram_dq_io & 0x00FF);
                        break;
                    case 2:
                        sdram_mem[addr] = (sdram_mem[addr] & 0x00FF) | (top->sdram_dq_io & 0xFF00);
                        break;
                    case 3:
                        sdram_mem[addr] = top->sdram_dq_io;
                        break;
                } 
            } else if (read_sdram) {
                //printf("Read at addr %x (%d), mask=%x (%x)\r\n", addr*2, burst_counter, mask, sdram_mem[addr]);
                top->sdram_dq_io = sdram_mem[addr];
            }

            if (read_sdram || write_sdram) {
                if (delay_burst == 0) {
                    if (burst_counter < 127)
                        burst_counter++;
                } else {
                    delay_burst--;
                }
            }
        }

        // if posedge clk
        if (top->clk) {

            if (contextp->time() < 5)
            {
                top->i_rst = 1; // Assert reset
            }
            else
            {
                top->i_rst = 0; // Deassert reset
            }

            if (top->o_ext_addr == 0) {
                // exit
                if (top->o_ext_stb) {
                    if (top->o_ext_we != 0) {
                        exit_value = top->o_ext_dat_w;
                        done = true;
                    }
                    top->i_ext_dat_r = 0;   // configuration simulator
                    top->i_ext_ack = 1;
                } else {
                    top->i_ext_ack = 0;
                }                
            } else if (top->o_ext_addr == 4) {
                // put char
                if (top->o_ext_stb) {
                    if (top->o_ext_we != 0) {
                        int c = top->o_ext_dat_w;
                        if (c == '\n') {
                            write(STDOUT_FILENO, "\r\n", 2);
                        } else {
                            write(STDOUT_FILENO, &c, 1);
                        }
                    }
                    top->i_ext_ack = 1;
                } else {
                    top->i_ext_ack = 0;
                }
            } else if (top->o_ext_addr == 8) {
                // read char
                if (top->o_ext_stb) {
                    uint8_t key = 0;

                    if (keys.size() > 0) {
                        key = keys.back();
                        keys.pop_back();
                    }
                    top->i_ext_dat_r = key;
                    top->i_ext_ack = 1;
                } else {
                    top->i_ext_ack = 0;
                }
            } else if (top->o_ext_addr == 0xC || top->o_ext_addr == 0x10) {
                // SD card
                if (top->o_ext_stb) {
                    if (top->o_ext_we != 0) {
                        if (top->o_ext_addr == 0xC) {
                            sdc_addr = top->o_ext_dat_w;
                        } else {
                            sdc_data.at(sdc_addr) = top->o_ext_dat_w;
                            sdc_addr++;
                        }
                    } else {
                        if (top->o_ext_addr == 0x10) {
                            top->i_ext_dat_r = sdc_data.at(sdc_addr);
                            sdc_addr++;
                        }
                    }
                    top->i_ext_ack = 1;
                } else {
                    top->i_ext_ack = 0;
                }
            }
        }

        contextp->timeInc(1);
        top->eval();
    }

    if (contextp->gotFinish()) {
        exit_value = 1;
    }
    // Final model cleanup
    top->final();

    printf("Exiting with value %d...\r\n", exit_value);
    exit(exit_value);
}
