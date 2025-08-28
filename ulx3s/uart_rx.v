// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Ref.: Project Oberon (https://www.projectoberon.net/)
//       (c) 2013 Niklaus Wirth (NW), Juerg Gutknecht (JG), Paul Reed (PR/PDR)

`default_nettype none

module uart_rx #(
    parameter FREQ_HZ = 25_000_000,
    parameter BAUD_RATE = 115_200
) (
    input wire i_clk,
    input wire i_rst,
    input wire i_rxd,
    input wire i_done,   // "byte has been read"
    output wire o_rdy,
    output wire [7:0] o_data
);

    wire endtick, midtick, endbit;
    wire [11:0] limit;
    reg run, stat;
    reg Q0, Q1;  // synchronizer and edge detector
    reg [11:0] tick;
    reg [3:0] bitcnt;
    reg [7:0] shreg;
    reg [3:0] inptr, outptr;
    reg [7:0] fifo [15:0];  // 16 byte buffer

    assign limit = 12'(FREQ_HZ / BAUD_RATE);
    assign endtick = tick == limit;
    assign midtick = tick == {1'b0, limit[11:1]};  // limit/2
    assign endbit = bitcnt == 8;
    assign o_data = fifo[outptr];
    assign o_rdy = ~(inptr == outptr);

    always @(posedge i_clk) begin
        Q0 <= i_rxd; Q1 <= Q0;
        run <= (Q1 & ~Q0) | ~(i_rst | endtick & endbit) & run;
        tick <= (run & ~endtick) ? tick+1 : 0;
        bitcnt <= (endtick & ~endbit) ? bitcnt + 1 :
            (endtick & endbit) ? 0 : bitcnt;
        shreg <= midtick ? {Q1, shreg[7:1]} : shreg;
        stat <= ~(inptr == outptr) | ~(i_rst | i_done) & stat;
        outptr <= i_rst ? 0 : o_rdy & i_done ? outptr+1 : outptr;
        inptr <= i_rst ? 0 : (endtick & endbit) ? inptr+1 : inptr;
        if (endtick & endbit) fifo[inptr] <= shreg;
    end
endmodule
