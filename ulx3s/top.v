// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module top(
    input        clk,
    input  [6:0] btn,
    output [7:0] led,
    input        uart_rx,
    output       uart_tx,
    input        sd_miso,
    output       sd_mosi,
    output       sd_sck,
    output       sd_ss,
    // SDRAM
    output        SDRAM_CLK,      // Clock for SDRAM chip
    output        SDRAM_CKE,      // Clock enabled
    inout  [15:0] SDRAM_DQ,       // Bidirectional data lines to/from SDRAM
    output [12:0] SDRAM_A,        // Address bus, multiplexed, 13 bits
    output [1:0]  SDRAM_BA,       // Bank select wires for 4 banks
    output [1:0]  SDRAM_DQM,      // Byte mask
    output        SDRAM_CSX,      // Chip select
    output        SDRAM_WEX,      // Write enable
    output        SDRAM_RASX,     // Row address select
    output        SDRAM_CASX      // Columns address select
);

    // Reset
    reg [7:0]	rst_cnt = 0;
    wire		rst = btn[1] || !(& rst_cnt);
    always @(posedge clk) begin
        rst_cnt <= rst_cnt + {6'd0,rst};
    end

    // External SBA bus
    wire [15:0] ext_addr;
    wire        ext_stb;
    wire [3:0]  ext_we;
    wire        ext_ack;
    wire [31:0] ext_dat_w, ext_dat_r;

    // SoC
    soc soc(
        .i_clk(clk),
        .i_rst(rst),
        // External bus
        .o_ext_addr(ext_addr),
        .o_ext_stb(ext_stb),
        .o_ext_we(ext_we),
        .i_ext_ack(ext_ack),
        .o_ext_dat_w(ext_dat_w),
        .i_ext_dat_r(ext_dat_r),
        // SDRAM
        .SDRAM_CLK(SDRAM_CLK),        // Clock for SDRAM chip
        .SDRAM_CKE(SDRAM_CKE),        // Clock enabled
        .SDRAM_D(SDRAM_DQ),           // Bidirectional data lines to/from SDRAM
        .SDRAM_ADDR(SDRAM_A),         // Address bus, multiplexed, 13 bits
        .SDRAM_BA(SDRAM_BA),          // Bank select wires for 4 banks
        .SDRAM_DQM(SDRAM_DQM),        // Byte mask
        .SDRAM_CS(SDRAM_CSX),         // Chip select
        .SDRAM_WE(SDRAM_WEX),         // Write enable
        .SDRAM_RAS(SDRAM_RASX),       // Row address select
        .SDRAM_CAS(SDRAM_CASX)        // Columns address select        
    );

    //
    // Devices on the external bus
    //

    wire addr_is_cfg  = ext_addr[15:12] == 4'h0;
    wire addr_is_led  = ext_addr[15:12] == 4'h1;
    wire addr_is_uart = ext_addr[15:12] == 4'h2;
    wire addr_is_spi  = ext_addr[15:12] == 4'h3;

    assign ext_ack = addr_is_cfg ? cfg_ack :
                     addr_is_led ? led_ack :
                     addr_is_uart ? uart_ack :
                     addr_is_spi ? spi_ack :
                     1'b0;

    assign ext_dat_r = addr_is_cfg ? cfg_dat_r :
                       addr_is_led ? led_dat_r :
                       addr_is_uart ? uart_dat_r :
                       addr_is_spi ? spi_dat_r :
                       32'd0;

    // Configuration

    wire cfg_ack;
    wire [31:0] cfg_dat_r;

    cfg cfg_dev(
        .i_stb(addr_is_cfg & ext_stb),
        .o_dat_r(cfg_dat_r),
        .o_ack(cfg_ack),
    );

    // LEDs

    wire led_ack;
    wire [31:0] led_dat_r;

    led led_dev(
        .i_clk(clk),
        .i_rst(rst),
        .i_stb(addr_is_led & ext_stb),
        .i_we(ext_we[0]),
        .o_ack(led_ack),
        .i_dat_w(ext_dat_w),
        .o_dat_r(led_dat_r),	
        .o_led(led)
    );

    // UART

    wire uart_ack;
    wire [31:0] uart_dat_r;

    uart uart_dev(
        .i_clk(clk),
        .i_rst(rst),
        .i_stb(addr_is_uart & ext_stb),
        .i_we(ext_we),
        .o_ack(uart_ack),
        .i_addr(ext_addr[2:0]),
        .i_dat_w(ext_dat_w),
        .o_dat_r(uart_dat_r),
        .o_tx(uart_tx),
        .i_rx(uart_rx),
        .o_int()
    );

    // SPI (SD Card)

    wire spi_ack;
    wire [31:0] spi_dat_r;

    spi spi_dev(
        .i_clk(clk),
        .i_rst(rst),
        .i_addr(ext_addr[3:0]),
        .i_stb(addr_is_spi & ext_stb),
        .i_we(ext_we[0]),
        .o_ack(spi_ack),
        .i_dat_w(ext_dat_w),
        .o_dat_r(spi_dat_r),
        .i_miso(sd_miso),
        .o_mosi(sd_mosi),
        .o_sck(sd_sck),
        .o_ss(sd_ss)
    );
   
endmodule
