// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module cursor_renderer #(
    // declarations for TV-simulator sync parameters
    // horizontal constants
    parameter H_DISPLAY       = 640, // horizontal display width
    parameter H_BACK          =  48, // horizontal left border (back porch)
    parameter H_FRONT         =  16, // horizontal right border (front porch)
    parameter H_SYNC          =  96, // horizontal sync width
    // vertical constants
    parameter V_DISPLAY       = 480, // vertical display height
    parameter V_BACK          =  33, // vertical top border
    parameter V_FRONT         =  10, // vertical bottom border
    parameter V_SYNC          =   2  // vertical sync # lines
) (
    input wire clk,
    input wire reset,
    input wire [9:0] hpos,
    input wire [9:0] vpos,
    output wire [3:0] col_index,
    input wire cursor_on,
    input wire [15:0] cursor_pos,
    input wire cursor_stb
);

    wire [9:0] hpos_a = hpos - H_BACK;
    wire [9:0] vpos_a = vpos - V_BACK;

    reg [15:0] row_base, col;
    wire [15:0] pos;
    reg [5:0] blink;


    wire visible_region = vpos >= V_BACK && vpos < V_BACK + V_DISPLAY && hpos >= H_BACK && hpos < H_BACK + H_DISPLAY;
    
    // lookup char and attr
    always @(posedge clk) begin
        if (reset) begin
            row_base <= 16'd0;
            blink <= 6'd0;
        end else begin

            if (cursor_stb)
                blink <= 6'd0;

            // if at the top
            if (hpos == 10'd0 && vpos == 10'd0) begin
                row_base <= 16'd0;
                blink <= blink + 6'd1;
            end

            // if inside the visible region
            if (visible_region) begin
                if (hpos_a[2:0] == 7) begin
                    col <= col + 16'd1;
                end
                if (vpos_a[2:0] == 7 && hpos_a == H_DISPLAY - 1)
                    row_base <= row_base + H_DISPLAY / 8;
            end else begin
                col <= 16'd0;
            end
        end
    end

    assign pos = row_base + col;
            
    assign col_index = (cursor_on && ~blink[5] && visible_region && pos == cursor_pos) ? 4'hF : 4'h0;
    
endmodule
