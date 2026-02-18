// Copyright (c) 2025-2026 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module top(
    input        clk,
    input  [1:0] button,
    output [4:0] led,
    input        usb_rx,
    output       usb_tx,
    input        sd_miso,
    output       sd_mosi,
    output       sd_clk,
    output       sd_csn,
    // SDRAM
    output        sdram_clk,      // Clock for SDRAM chip
    output        sdram_cke,      // Clock enabled
    inout  [15:0] sdram_dq,       // Bidirectional data lines to/from SDRAM
    output [12:0] sdram_a,        // Address bus, multiplexed, 13 bits
    output [1:0]  sdram_ba,       // Bank select wires for 4 banks
    output [1:0]  sdram_dqm,      // Byte mask
    output        sdram_csn,      // Chip select
    output        sdram_wen,      // Write enable
    output        sdram_rasn,     // Row address select
    output        sdram_casn,     // Columns address select
    // GPDI
    output [3:0] gpdi_dp,
    output [3:0] gpdi_dn,
    // USB
    output      logic [1:0] usb_pull_dp, usb_pull_dn,
    inout       logic [1:0] usb_dp, usb_dn
);

    assign usb_pull_dp = 2'b11; 	// pull USB D+ to +3.3V through 1.5K resistor
    assign usb_pull_dn = 2'b11; 	// pull USB D- to +3.3V through 1.5K resistor

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
    wire		rst = ~button[0] || !(& rst_cnt) || !pll_locked || !pll_video_locked;
    always @(posedge clk) begin
        rst_cnt <= rst_cnt + {6'd0,rst};
    end

    wire [7:0] leds;
    assign led = leds[4:0];

    soc_ext soc_ext(
        .sys_clk(sys_clk),
        .sdr_clk(sdr_clk),
        .pix_x5_clk(pix_x5_clk),
        .i_rst(rst),
        .o_sdram_clk(sdram_clk),
        .o_sdram_cke(sdram_cke),
        .io_sdram_dq(sdram_dq),
        .o_sdram_a(sdram_a),
        .o_sdram_ba(sdram_ba),
        .o_sdram_dqm(sdram_dqm),
        .o_sdram_csn(sdram_csn),
        .o_sdram_wen(sdram_wen),
        .o_sdram_rasn(sdram_rasn),
        .o_sdram_casn(sdram_casn),
        .o_led(leds),
        .i_uart_rx(usb_rx),
        .o_uart_tx(usb_tx),
        .i_sd_miso(sd_miso),
        .o_sd_mosi(sd_mosi),
        .o_sd_clk(sd_clk),
        .o_sd_csn(sd_csn),
        .i_ps2_kbd_clk(usb_dp[0]),
        .i_ps2_kbd_data(usb_dn[0]),
        .o_gpdi_dp(gpdi_dp),
        .o_gpdi_dn(gpdi_dn)
    );
   
endmodule
