// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module soc(
    input i_clk,
    input i_rst,
    // External bus
    output logic [15:0] o_ext_addr,
    output logic        o_ext_stb,
    output logic [3:0]  o_ext_we,
    input  logic        i_ext_ack,
    output logic [31:0] o_ext_dat_w,
    input  logic [31:0] i_ext_dat_r
);

    // SBA Simple Bus Architecture
    wire		sba_rst = i_rst;
    wire		sba_clk = i_clk;
    wire  [3:0] sba_we;
    wire        sba_stb;
    wire [31:0] sba_addr;
    wire [31:0] sba_dat_r;
    wire [31:0] sba_dat_w;
    wire        sba_ack;

    wire addr_is_rom = (sba_addr[31:28]==4'h0);
    wire addr_is_bram = (sba_addr[31:28]==4'h1);
    wire addr_is_ext = (sba_addr[31:28]==4'h2);
    wire addr_is_timer = (sba_addr[31:28]==4'h3);

    assign sba_ack = addr_is_rom ? rom_ack :
                     addr_is_bram ? bram_ack :
                     addr_is_ext ? i_ext_ack :
                     addr_is_timer ? timer_ack :
                     1'b0;

    assign sba_dat_r = addr_is_rom ? rom_dat_r :
                       addr_is_bram ? bram_dat_r :
                       addr_is_ext ? i_ext_dat_r :
                       addr_is_timer ? timer_dat_r :
                       32'd0;

    // OR32 CPU
    or32 or32(
        .i_rst(sba_rst),
        .i_clk(sba_clk),
        .o_addr(sba_addr),
        .o_dat_w(sba_dat_w),
        .o_we(sba_we),
        .i_dat_r(sba_dat_r),
        .o_stb(sba_stb),
        .i_ack(sba_ack)
    );

    // 32 KiB of ROM preloaded with boot loader
    reg [31:0] ROM[8192];
    initial $readmemh("bios.hex", ROM); 
    wire rom_stb = addr_is_rom & sba_stb;
    wire [12:0] rom_addr = sba_addr[14:2];
    reg [31:0] rom_dat_r;
    always @(posedge i_clk)
        rom_dat_r = ROM[rom_addr];
    reg rom_ack;
    always @(posedge i_clk)
        if (rom_stb) rom_ack <= 1;
        else rom_ack <= 0;
    
    // 256 KiB of BRAM preloaded with the OS kernel
    reg [31:0] BRAM[65536];
    initial $readmemh("kernel.hex", BRAM);
    wire bram_stb = addr_is_bram & sba_stb;
    wire [15:0] bram_addr = sba_addr[17:2];
    always @(posedge i_clk) begin
        if(sba_we[0] & bram_stb) BRAM[bram_addr][ 7:0 ] <= sba_dat_w[ 7:0 ];
        if(sba_we[1] & bram_stb) BRAM[bram_addr][15:8 ] <= sba_dat_w[15:8 ];
        if(sba_we[2] & bram_stb) BRAM[bram_addr][23:16] <= sba_dat_w[23:16];
        if(sba_we[3] & bram_stb) BRAM[bram_addr][31:24] <= sba_dat_w[31:24];
    end
    reg [31:0] bram_dat_r;
    always @(posedge i_clk)
        bram_dat_r = BRAM[bram_addr];
    reg bram_ack;
    always @(posedge i_clk)
        if (bram_stb) bram_ack <= 1;
        else bram_ack <= 0;

    // External bus
    assign o_ext_addr  = sba_addr[15:0];
    assign o_ext_dat_w = sba_dat_w;
    assign o_ext_we    = sba_we;
    assign o_ext_stb   = addr_is_ext & sba_stb;

    // Timer
    wire [31:0] timer_dat_r;
    wire timer_ack;
    timer timer(
        .i_clk(i_clk),
        .i_rst(i_rst),
        .i_addr(sba_addr[3:0]),
        .i_stb(addr_is_timer & sba_stb),
        .o_ack(timer_ack),
        .o_dat_r(timer_dat_r)
    );     

endmodule
