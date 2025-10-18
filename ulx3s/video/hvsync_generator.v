// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module hvsync_generator #(
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
    output reg hsync,
    output reg vsync,
    output wire display_on,
    output reg [9:0] hpos,
    output reg [9:0] vpos
);

    // derived constants
    localparam H_SYNC_START    = H_BACK + H_DISPLAY + H_FRONT;
    localparam H_SYNC_END      = H_BACK + H_DISPLAY + H_FRONT + H_SYNC - 1;
    localparam H_MAX           = H_BACK + H_DISPLAY + H_FRONT + H_SYNC - 1;
    localparam V_SYNC_START    = V_BACK + V_DISPLAY + V_FRONT;
    localparam V_SYNC_END      = V_BACK + V_DISPLAY + V_FRONT + V_SYNC - 1;
    localparam V_MAX           = V_BACK + V_DISPLAY + V_FRONT + V_SYNC - 1;

    wire hmaxxed = (hpos == H_MAX) || reset;	// set when hpos is maximum
    wire vmaxxed = (vpos == V_MAX) || reset;	// set when vpos is maximum
    
    // horizontal position counter
    always @(posedge clk)
    begin
        hsync <= (hpos >= H_SYNC_START && hpos <= H_SYNC_END);
        if(hmaxxed)
            hpos <= 0;
        else
            hpos <= hpos + 1;
    end

    // vertical position counter
    always @(posedge clk)
    begin
        vsync <= (vpos >= V_SYNC_START && vpos <= V_SYNC_END);
        if(hmaxxed)
            if (vmaxxed)
                vpos <= 0;
            else
                vpos <= vpos + 1;
    end
    
    // display_on is set when beam is in "safe" visible frame
    assign display_on = (hpos >= H_BACK && hpos < (H_BACK + H_DISPLAY)) && (vpos >= V_BACK && vpos < (V_BACK + V_DISPLAY));

endmodule
