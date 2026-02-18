// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module vram #(
    parameter A = 10  // # of address bits
) (
    input wire clk,             // clock
    input wire [A-1:0] addr,    // address
    input wire [31:0] din,	    // data input
    input wire [3:0] we,        // write enable
    output reg [31:0] dout	    // data output
);
    
    
    reg [31:0] mem [0:(1<<A)-1]; // (1<<A)x32 bit memory
    
    always @(posedge clk) begin
        if (we[0])
            mem[addr][7:0] <= din[7:0];
        if (we[1])
            mem[addr][15:8] <= din[15:8];
        if (we[2])
            mem[addr][23:16] <= din[23:16];
        if (we[3])
            mem[addr][31:24] <= din[31:24];
        dout <= mem[addr];	// read memory to dout (sync)
    end

endmodule
