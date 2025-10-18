// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module vdu (
    input wire i_rst,
    input wire i_clk,
    input wire [16:0] i_addr,
    input wire i_stb,
    input wire [3:0] i_we,
    output reg o_ack,
    input wire [31:0] i_dat_w,
    output wire [31:0] o_dat_r,
    output wire o_int,
    // video
    output wire o_vga_hsync,
    output wire o_vga_vsync,
    output wire o_vga_de,
    output wire [7:0] o_vga_r,
    output wire [7:0] o_vga_g,
    output wire [7:0] o_vga_b
);

    assign o_int = vsync;

    wire [9:0] hpos;
    wire [9:0] vpos;

    wire hsync, vsync;
    wire [3:0] col_index;

    assign o_vga_hsync = ~hsync;
    assign o_vga_vsync = ~vsync;

    // palette
    logic [11:0] palette[16];

    initial begin
        $readmemh("vdu_palette.hex", palette);
    end	

    // video sync generator
    hvsync_generator hvsync_gen(
        .clk(i_clk),
        .reset(i_rst),
        .hsync(hsync),
        .vsync(vsync),
        .display_on(o_vga_de),
        .hpos(hpos),
        .vpos(vpos)
    );

    // video RAM
    wire [31:0] ram_read;
    reg [31:0] ram_write;
    reg [3:0] ram_write_enable;

    vram #(
        .A(14)
    ) vram(
        .clk(i_clk),
        .dout(ram_read),
        .din(ram_write),
        .addr(mux_ram_addr),
        .we(ram_write_enable)
    );

    // tile ROM
    wire [10:0] tile_rom_addr;
    wire [7:0] tile_rom_data;

    font_cp437_8x8 tile_rom(
        .addr(tile_rom_addr),
        .data(tile_rom_data)
    );

      // tile graphics
    reg [15:0] tile_ram_addr;
    wire tile_reading;
    wire [3:0] tile_col_index;

    tile_renderer tile_gen(
        .clk(i_clk),
        .reset(i_rst),
        .hpos(hpos),
        .vpos(vpos),
        .ram_addr(tile_ram_addr),
        .ram_read(ram_read),
        .ram_busy(tile_reading),
        .rom_addr(tile_rom_addr),
        .rom_data(tile_rom_data),
        .col_index(tile_col_index)
    );

    wire [3:0] cursor_col_index;
    reg cursor_on;
    reg [15:0] cursor_pos;
    reg cursor_stb;

    cursor_renderer cursor_gen(
        .clk(i_clk),
        .reset(i_rst),
        .hpos(hpos),
        .vpos(vpos),
        .col_index(cursor_col_index),
        .cursor_on(cursor_on),
        .cursor_pos(cursor_pos),
        .cursor_stb(cursor_stb)
    );

    assign col_index = tile_col_index | cursor_col_index;

    wire [3:0] vga_r, vga_g, vga_b;
    assign vga_r = o_vga_de ? palette[col_index][11:8] : 4'h0;
    assign vga_g = o_vga_de ? palette[col_index][7:4] : 4'h0;
    assign vga_b = o_vga_de ? palette[col_index][3:0] : 4'h0;

    assign o_vga_r = {vga_r, vga_r};
    assign o_vga_g = {vga_g, vga_g};
    assign o_vga_b = {vga_b, vga_b};

    wire [13:0] mux_ram_addr; // 14-bit RAM access
    assign mux_ram_addr = cpu_accessing ? i_addr[15:2] : tile_ram_addr[13:0];

    var [31:0] reg_read;
    always @(*) begin
        case (i_addr[2]) 
            1'b0: reg_read = {31'd0, cursor_on};
            1'b1: reg_read = {16'd0, cursor_pos};
        endcase
    end

    assign o_dat_r = i_addr[16] ? reg_read : ram_read;

    reg access_pending, cpu_accessing;
    always @(posedge i_clk) begin
        if (i_rst) begin
            o_ack <= 1'b0;
            access_pending <= 1'b0;
            cpu_accessing <= 1'b0;
            cursor_on <= 1'b0;
            cursor_pos <= 16'd0;
            cursor_stb <= 1'b0;
        end else begin
            o_ack <= 1'b0;
            ram_write_enable <= 4'd0;
            cursor_stb <= 1'b0;
            if (cpu_accessing) begin
                o_ack <= 1'b1;
                cpu_accessing <= 1'b0;
            end
            if (i_stb || access_pending) begin
                if (i_addr[16]) begin
                    // register access
                    if (|i_we) begin
                        case (i_addr[2]) 
                            1'b0: cursor_on <= i_dat_w[0];
                            1'b1: cursor_pos <= i_dat_w[15:0];
                        endcase
                        cursor_stb <= 1'b1;
                    end
                    o_ack <= 1'b1;
                end else begin
                    // VRAM access
                    access_pending <= 1'b1;
                    if (!tile_reading) begin
                        cpu_accessing <= 1'b1;
                        ram_write <= i_dat_w;
                        ram_write_enable <= i_we;
                        access_pending <= 1'b0;
                    end
                end
            end
        end
    end

endmodule
