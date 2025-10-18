// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Ref.: Project Oberon (https://www.projectoberon.net/)
//       (c) 2013 Niklaus Wirth (NW), Juerg Gutknecht (JG), Paul Reed (PR/PDR)

`default_nettype none

module ps2_kbd(
    input i_clk,
    input i_rst,
    input i_done,   // "byte has been read"
    output o_rdy,   // "byte is available"
    output [7:0] o_data,
    input i_ps2_clk,   // serial input
    input i_ps2_data
);
	 
    reg Q0, Q1;  // synchronizer and falling edge detector
    reg [10:0] shreg;
    reg [3:0] inptr, outptr;
    reg [7:0] fifo [15:0];  // 16 byte buffer
    wire endbit;
    wire shift;

    assign endbit = ~shreg[0];  //start bit reached correct pos
    assign shift = Q1 & ~Q0;
    assign o_data = fifo[outptr];
    assign o_rdy = ~(inptr == outptr);

    always @(posedge i_clk) begin
        Q0 <= i_ps2_clk; Q1 <= Q0;
        shreg <= (i_rst | endbit) ? 11'h7FF :
            shift ? {i_ps2_data, shreg[10:1]} : shreg;
        outptr <= i_rst ? 0 : o_rdy & i_done ? outptr + 1 : outptr;
        inptr <= i_rst ? 0 : endbit ? inptr + 1 : inptr;
        if (endbit) fifo[inptr] <= shreg[8:1];
    end	 
endmodule
