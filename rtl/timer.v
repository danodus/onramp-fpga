// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Timer
// 0x00: Number of milliseconds (low)
// 0x04: Number of milliseconds (high)

`default_nettype none

module timer #(
    parameter FREQ_HZ = 25_000_000
) (
    input wire i_clk,
    input wire i_rst,
    input wire [3:0] i_addr,
    input wire i_stb,
    output wire [31:0] o_dat_r,
    output wire o_ack
);

    reg [15:0] counter;
    reg [63:0] milliseconds;

    always @(posedge i_clk) begin
        if (i_rst) begin
            counter <= 16'd0;
            milliseconds <= 64'd0;
        end else begin
            if (counter >= 16'(FREQ_HZ / 1_000)) begin
                milliseconds <= milliseconds + 64'd1;
                counter <= 16'd0;
            end else begin
                counter <= counter + 16'd1;
            end
        end
    end

    assign o_ack = i_stb;

    assign o_dat_r = (i_addr == 4'd0) ? milliseconds[31:0] :
                     (i_addr == 4'd4) ? milliseconds[63:32] :
                     32'd0;

endmodule
