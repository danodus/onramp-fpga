// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Ref.: Project Oberon (https://www.projectoberon.net/)
//       (c) 2013 Niklaus Wirth (NW), Juerg Gutknecht (JG), Paul Reed (PR/PDR)

`default_nettype none

module uart_tx #(
    parameter FREQ_HZ = 25_000_000,
    parameter BAUD_RATE = 115_200
) (
    input wire i_clk,
    input wire i_rst,
    input wire i_start, // request to accept and send a byte
    input wire [7:0] i_data,
    output wire o_rdy,
    output wire o_txd
);

    wire endtick, endbit;
    wire [11:0] limit;
    reg run;
    reg [11:0] tick;
    reg [3:0] bitcnt;
    reg [8:0] shreg;

    assign limit = 12'(FREQ_HZ / BAUD_RATE);
    assign endtick = tick == limit;
    assign endbit = bitcnt == 9;
    assign o_rdy = ~run;
    assign o_txd = shreg[0];

    always @ (posedge i_clk) begin
        run <= (i_rst | endtick & endbit) ? 0 : i_start ? 1 : run;
        tick <= (run & ~endtick) ? tick + 1 : 0;
        bitcnt <= (endtick & ~endbit) ? bitcnt + 1 :
            (endtick & endbit) ? 0 : bitcnt;
        shreg <= (i_rst) ? 1 : i_start ? {i_data, 1'b0} :
            endtick ? {1'b1, shreg[8:1]} : shreg;
    end
endmodule
