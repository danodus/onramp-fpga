// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module ps2 (
    input wire i_rst,
    input wire i_clk,
    input wire [2:0] i_addr,
    input wire i_stb,
    input wire [3:0] i_we,
    output wire o_ack,
    input wire [31:0] i_dat_w,
    output wire [31:0] o_dat_r,
    output wire o_int,
    // keyboard
    input wire i_ps2_kbd_clk,
    input wire i_ps2_kbd_data
);

    assign o_int = 1'b0;
    assign o_ack = i_stb;

    wire [7:0] kbd_data;
    wire kbd_done, kbd_rdy;
    
    ps2_kbd PS2_KBD(
        .i_clk(i_clk),
        .i_rst(i_rst),
        .i_done(kbd_done),
        .o_rdy(kbd_rdy),
        .o_data(kbd_data),
        .i_ps2_clk(i_ps2_kbd_clk),
        .i_ps2_data(i_ps2_kbd_data)
    );

    assign kbd_done = !i_we[0] & i_stb & (i_addr == 3'd0);

    assign o_dat_r = (i_addr == 3'd0) ? {24'b0, kbd_data} :
                     (i_addr == 3'd4) ? {31'b0, kbd_rdy} :
                     32'd0;


endmodule
