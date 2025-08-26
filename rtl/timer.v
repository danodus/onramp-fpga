// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Timer
// 0x00: Number of seconds (low)
// 0x04: Number of seconds (high)
// 0x08: Number of nanoseconds

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

    reg [31:0] nanoseconds;
    reg [63:0] seconds;

    always @(posedge i_clk) begin
        if (i_rst) begin
            nanoseconds <= 32'd0;
            seconds <= 64'd0;
        end else begin
            nanoseconds <= nanoseconds + 32'(1_000_000_000 / FREQ_HZ);
            if (nanoseconds >= 32'd1_000_000_000) begin
                nanoseconds <= 32'd0;
                seconds <= seconds + 64'd1;
            end
        end
    end

    assign o_ack = i_stb;

    assign o_dat_r = (i_addr == 4'd0) ? seconds[31:0] :
                     (i_addr == 4'd4) ? seconds[63:32] :
                     (i_addr == 4'd8) ? nanoseconds :
                     32'd0;

endmodule
