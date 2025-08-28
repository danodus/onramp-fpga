// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module uart #(
    parameter FREQ_HZ = 25_000_000,
    parameter BAUD_RATE = 115_200
) (
    input wire i_rst,
    input wire i_clk,
    input wire [2:0] i_addr,
    input wire i_stb,
    input wire [3:0] i_we,
    output wire o_ack,
    input wire [31:0] i_dat_w,
    output wire [31:0] o_dat_r,
    output wire o_tx,
    input wire i_rx,
    output wire o_int
);

    assign o_int = 1'b0;
    assign o_ack = i_stb;
    
    wire [7:0] data_tx, data_rx;
    wire rdy_rx, done_rx, start_tx, rdy_tx;

    uart_rx #(
        .FREQ_HZ(FREQ_HZ),
        .BAUD_RATE(BAUD_RATE)
    ) UART_RX(
        .i_clk(i_clk),
        .i_rst(i_rst),
        .i_rxd(i_rx),
        .i_done(done_rx),
        .o_rdy(rdy_rx),
        .o_data(data_rx)
    );

    uart_tx #(
        .FREQ_HZ(FREQ_HZ),
        .BAUD_RATE(BAUD_RATE)
    ) UART_TX(
        .i_clk(i_clk),
        .i_rst(i_rst),
        .i_start(start_tx),
        .i_data(data_tx),
        .o_rdy(rdy_tx),
        .o_txd(o_tx)
    );

    assign data_tx = i_dat_w[7:0];
    assign start_tx = i_we[0] & i_stb & (i_addr == 3'd0);
    assign done_rx = !i_we[0] & i_stb & (i_addr == 3'd0);

    assign o_dat_r = (i_addr == 3'd0) ? {24'b0, data_rx} :
                     (i_addr == 3'd4) ? {30'b0, rdy_tx, rdy_rx} :
                     32'd0;

endmodule
