// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module soc(
    input i_clk,
    input i_rst,
    // External bus
    output [15:0] o_ext_addr,
    output        o_ext_stb,
    output [3:0]  o_ext_we,
    input         i_ext_ack,
    output [31:0] o_ext_dat_w,
    input  [31:0] i_ext_dat_r,
`ifdef SDRAM
    // SDRAM	
    output        SDRAM_CLK,        // Clock for SDRAM chip
    output        SDRAM_CKE,        // Clock enabled
    inout  [15:0] SDRAM_D,          // Bidirectional data lines to/from SDRAM
    output [12:0] SDRAM_ADDR,       // Address bus, multiplexed, 13 bits
    output [1:0]  SDRAM_BA,         // Bank select wires for 4 banks
    output [1:0]  SDRAM_DQM,        // Byte mask
    output        SDRAM_CS,         // Chip select
    output        SDRAM_WE,         // Write enable
    output        SDRAM_RAS,        // Row address select
    output        SDRAM_CAS         // Columns address select
`endif
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
    wire addr_is_ram = (sba_addr[31:28]==4'h1);
    wire addr_is_ext = (sba_addr[31:28]==4'h2);
    wire addr_is_timer = (sba_addr[31:28]==4'h3);

    assign sba_ack = addr_is_rom ? rom_ack :
                     addr_is_ram ? ram_ack :
                     addr_is_ext ? i_ext_ack :
                     addr_is_timer ? timer_ack :
                     1'b0;

    assign sba_dat_r = addr_is_rom ? rom_dat_r :
                       addr_is_ram ? ram_dat_r :
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

    // 64 KiB of ROM preloaded with boot loader
    reg [31:0] ROM[16384];
    initial $readmemh("bios.hex", ROM); 
    wire rom_stb = addr_is_rom & sba_stb;
    wire [13:0] rom_addr = sba_addr[15:2];
    reg [31:0] rom_dat_r;
    always @(posedge i_clk)
        rom_dat_r = ROM[rom_addr];
    reg rom_ack;
    always @(posedge i_clk)
        if (rom_stb) begin
`ifndef SYNTHESIS
            if (|sba_we)
                $display("*** Write to ROM: value %x at address %x\r", sba_dat_w, sba_addr);
`endif            
            rom_ack <= 1;
        end
        else rom_ack <= 0;

    //
    // RAM (32 MiB)
    //    

    wire ram_ack;
    reg [31:0] ram_dat_r;

`ifdef SDRAM

    // SDRAM

    wire [24:0] sdram_addr = sba_addr[24:0];
    wire sdram_busy;
    wire sdram_stb = addr_is_ram & sba_stb;
    reg sdram_pending;

    always @(posedge i_clk)
        if (i_rst) sdram_pending <= 0;
        else if (sdram_stb & sdram_busy) sdram_pending <= 1;
        else if (sdram_pending & ~sdram_busy) sdram_pending <= 0;

    reg sdram_done;
    always @(posedge i_clk)
        if (i_rst) sdram_done <= 0;
        else if (sdram_stb & ~sdram_busy) sdram_done <= 1;
        else if (sdram_done & ~sdram_busy) sdram_done <= 0;

    wire sdram_go = ((sdram_stb & ~sdram_busy) | (sdram_pending));
    wire [3:0] sdram_wmask = sdram_go? sba_we : 4'b0;
    wire sdram_rd = (sdram_go &~ (|sba_we));
    assign ram_ack = (sdram_pending & ~sdram_busy) | (sdram_done & ~sdram_busy);

    sdram SDRAM(
        .clk(i_clk),
        .resetn(~i_rst),
        .wmask(sdram_wmask),
        .rd(sdram_rd),
        .addr(sdram_addr),
        .din(sba_dat_w),
        .dout(ram_dat_r),
        .busy(sdram_busy),
        .sd_clk(SDRAM_CLK),        // Clock for SDRAM chip
        .sd_cke(SDRAM_CKE),        // Clock enabled
        .sd_d(SDRAM_D),          // Bidirectional data lines to/from SDRAM
        .sd_addr(SDRAM_ADDR),       // Address bus, multiplexed, 13 bits
        .sd_ba(SDRAM_BA),         // Bank select wires for 4 banks
        .sd_dqm(SDRAM_DQM),        // Byte mask
        .sd_cs(SDRAM_CS),         // Chip select
        .sd_we(SDRAM_WE),         // Write enable
        .sd_ras(SDRAM_RAS),        // Row address select
        .sd_cas(SDRAM_CAS)        // Columns address select
    );

`else // SDRAM

    // 32 MiB of BRAM preloaded with the OS shell
    
    reg [31:0] BRAM[32*1024*1024/4];
`ifndef SYNTHESIS    
    initial $readmemh("shell.hex", BRAM);
`endif
    wire bram_stb = addr_is_ram & sba_stb;
    wire [22:0] bram_addr = sba_addr[24:2];
    always @(posedge i_clk) begin
        if(sba_we[0] & bram_stb) BRAM[bram_addr][ 7:0 ] <= sba_dat_w[ 7:0 ];
        if(sba_we[1] & bram_stb) BRAM[bram_addr][15:8 ] <= sba_dat_w[15:8 ];
        if(sba_we[2] & bram_stb) BRAM[bram_addr][23:16] <= sba_dat_w[23:16];
        if(sba_we[3] & bram_stb) BRAM[bram_addr][31:24] <= sba_dat_w[31:24];
    end
    reg bram_ack;
    always @(posedge i_clk)
        ram_dat_r = BRAM[bram_addr];
    always @(posedge i_clk)
        if (bram_stb) bram_ack <= 1;
        else bram_ack <= 0;
    assign ram_ack = bram_ack;

`endif // SDRAM

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
