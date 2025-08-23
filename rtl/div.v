// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Ref.: https://projectf.io/posts/division-in-verilog/

module div #(parameter WIDTH=32) (  // width of numbers in bits
    input                  i_rst,   // reset
    input                  i_clk,   // clock
    input                  i_start, // start calculation
    output reg             o_busy,  // calculation in progress
    output reg             o_done,  // calculation is complete (high for one tick)
    output reg             o_valid, // result is valid
    output reg             o_dbz,   // divide by zero
    input      [WIDTH-1:0] i_a,     // dividend (numerator)
    input      [WIDTH-1:0] i_b,     // divisor (denominator)
    output reg [WIDTH-1:0] o_val,   // result value: quotient
    output reg [WIDTH-1:0] o_rem    // result: remainder
);

    reg [WIDTH-1:0] b1;             // copy of divisor
    reg [WIDTH-1:0] quo, quo_next;  // intermediate quotient
    reg [WIDTH:0] acc, acc_next;    // accumulator (1 bit wider)
    reg [$clog2(WIDTH)-1:0] i;      // iteration counter

    // division algorithm iteration
    always @(*) begin
        if (acc >= {1'b0, b1}) begin
            acc_next = acc - b1;
            {acc_next, quo_next} = {acc_next[WIDTH-1:0], quo, 1'b1};
        end else begin
            {acc_next, quo_next} = {acc, quo} << 1;
        end
    end

    // calculation control
    always @(posedge i_clk) begin
        o_done <= 0;
        if (i_start) begin
            o_valid <= 0;
            i <= 0;
            if (i_b == 0) begin  // catch divide by zero
                o_busy <= 0;
                o_done <= 1;
                o_dbz <= 1;
            end else begin
                o_busy <= 1;
                o_dbz <= 0;
                b1 <= i_b;
                {acc, quo} <= {{WIDTH{1'b0}}, i_a, 1'b0};  // initialize calculation
            end
        end else if (o_busy) begin
            if (i == {$clog2(WIDTH){1'b1}}) begin  // we're done
                o_busy <= 0;
                o_done <= 1;
                o_valid <= 1;
                o_val <= quo_next;
                o_rem <= acc_next[WIDTH:1];  // undo final shift
            end else begin  // next iteration
                i <= i + 1;
                acc <= acc_next;
                quo <= quo_next;
            end
        end
        if (i_rst) begin
            o_busy <= 0;
            o_done <= 0;
            o_valid <= 0;
            o_dbz <= 0;
            o_val <= 0;
            o_rem <= 0;
        end
    end
endmodule