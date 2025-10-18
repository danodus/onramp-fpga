// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Displays a 80x60 grid of 8x8 tiles, whose attributes are
// fetched from RAM, and whose bitmap patterns are in ROM.

`default_nettype none

module tile_renderer #(
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
    output reg [15:0] ram_addr,
    input wire [31:0] ram_read,
    output reg ram_busy,
    // font
    output wire [10:0] rom_addr,
    input wire [7:0] rom_data
);

    // start loading cells from RAM at this hpos value
    localparam HLOAD = H_FRONT + H_DISPLAY + H_FRONT;
    localparam HLOAD_CNT = H_DISPLAY / 8 / 2;

    wire [9:0] hpos_a = hpos - H_BACK;
    wire [9:0] vpos_a = vpos - V_BACK;

    reg [7:0] page_base = 8'h7e;	// page table base (8 bits)
    reg [15:0] cell_addr;		    // cell address (16 bits)
    reg [6:0] col_load;
    reg [5:0] row;
    wire [6:0] col = hpos_a[9:3];	// 7-bit column, hpos / 8
    wire [2:0] yofs = vpos_a[2:0];  // scanline of cell (0-7)
    wire [2:0] xofs = hpos_a[2:0];  // which pixel to draw (0-7)
    
    reg [15:0] cur_cell;
    wire [7:0] cur_char = cur_cell[7:0];
    wire [7:0] cur_attr = cur_cell[15:8];

    // tile ROM address
    assign rom_addr = {cur_char, yofs};
    
    reg [15:0] row_buffer[0:79];
    
    // lookup char and attr
    always @(posedge clk) begin
        // reset row to 0 when last row displayed
        if (vpos <= V_BACK - 8 || vpos > V_BACK + V_DISPLAY) begin
            cell_addr <= 16'd0;
            row <= 0;
        end
        // time to read a row?
        if (vpos >= V_BACK - 8 && vpos < V_BACK + V_DISPLAY && hpos >= H_BACK) begin
            if (vpos_a[2:0] == 7) begin
                // read row_base from page table (2 bytes)
                case (hpos)
                    // assert busy 5 cycles before first RAM read
                    HLOAD - 5: ram_busy <= 1;
                    // deassert BUSY and increment row counter
                    HLOAD + HLOAD_CNT + 2: begin
                        ram_busy <= 0;
                        row <= row + 1;
                    end
                endcase
                // load row of tile data from RAM
                // (last two twice)
                if (hpos >= HLOAD && hpos < HLOAD + HLOAD_CNT + 2) begin
                    // set address bus to
                    ram_addr <= cell_addr;
                    if (hpos < HLOAD + HLOAD_CNT)
                        cell_addr <= cell_addr + 16'd1;
                    // store value on data bus from (row_base + hpos - 2)
                    // which was read two cycles ago
                    if (hpos >= HLOAD + 2) begin
                        row_buffer[col_load] <= ram_read[15:0];
                        row_buffer[col_load + 1] <= ram_read[31:16];
                        col_load <= col_load + 7'd2;
                    end
                end else begin
                    col_load <= 7'd0;
                end
            end
            // latch character data
            if (hpos < H_BACK + H_DISPLAY) begin
                case (hpos[2:0])
                    7: begin
                        // read next cell
                        cur_cell <= row_buffer[col+1];
                    end
                endcase
            end else if (hpos == H_BACK + H_DISPLAY + H_FRONT + H_SYNC - 1) begin
                // read first cell of next row
                cur_cell <= row_buffer[0];
            end
        end
    end
            
    // extract bit from ROM output
    assign col_index = rom_data[~xofs] ? cur_attr[3:0] : cur_attr[7:4];
    
endmodule
