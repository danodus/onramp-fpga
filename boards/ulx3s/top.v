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
    output        SDRAM_CASX,     // Columns address select
    // GPDI
    output [3:0] gpdi_dp,
    output [3:0] gpdi_dn,
    // GPIO (PS/2)
    inout [27:0] gp, gn           // GPIO Header pins available as one data block
);

    wire sys_clk, sdr_clk, pix_x5_clk, pll_locked, pll_video_locked;
    pll pll(
        .clkin(clk),
        .clkout0(sdr_clk),
        .clkout1(sys_clk),
        .locked(pll_locked)
    );

    pll_video pll_video(
        .clkin(clk),
        .clkout0(pix_x5_clk),
        .locked(pll_video_locked)
    );

    // Reset
    reg [7:0]	rst_cnt = 0;
    wire		rst = ~btn[0] || !(& rst_cnt) || !pll_locked || !pll_video_locked;
    always @(posedge clk) begin
        rst_cnt <= rst_cnt + {6'd0,rst};
    end

    soc_ext soc_ext(
        .sys_clk(sys_clk),
        .sdr_clk(sdr_clk),
        .pix_x5_clk(pix_x5_clk),
        .i_rst(rst),
        .o_sdram_clk(SDRAM_CLK),
        .o_sdram_cke(SDRAM_CKE),
        .io_sdram_dq(SDRAM_DQ),
        .o_sdram_a(SDRAM_A),
        .o_sdram_ba(SDRAM_BA),
        .o_sdram_dqm(SDRAM_DQM),
        .o_sdram_csn(SDRAM_CSX),
        .o_sdram_wen(SDRAM_WEX),
        .o_sdram_rasn(SDRAM_RASX),
        .o_sdram_casn(SDRAM_CASX),
        .o_led(led),
        .i_uart_rx(uart_rx),
        .o_uart_tx(uart_tx),
        .i_sd_miso(sd_miso),
        .o_sd_mosi(sd_mosi),
        .o_sd_clk(sd_sck),
        .o_sd_csn(sd_ss),
        .i_ps2_kbd_clk(gn[1]),
        .i_ps2_kbd_data(gn[3]),
        .o_gpdi_dp(gpdi_dp),
        .o_gpdi_dn(gpdi_dn)
    );

endmodule
