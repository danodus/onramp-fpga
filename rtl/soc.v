// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module soc #(
    parameter FREQ_HZ = 25_000_000
) (
    input i_clk,
    input i_clk_sdram,
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

    wire ce;

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
        .i_ce(ce),
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
            if (|sba_we) begin
                $display("*** Write to ROM: value %x at address %x\r", sba_dat_w, sba_addr);
                $finish;
            end
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

    wire ram_stb = addr_is_ram & sba_stb;

    reg  [1:0]  cntrl0_user_command_register;
    wire [15:0] cntrl0_user_input_data;
    wire [15:0] sys_DOUT;
    wire        sys_rd_data_valid;
    wire        sys_wr_data_valid;
    wire [1:0]  sys_cmd_ack;
    reg         crw = 1'b0;
    wire [17:0] waddr;

    reg [22:0] sys_addr;

    always @(*) begin
        sys_addr = 23'hxxxxx;
        case(cntrl0_user_command_register)
            2'b01: sys_addr = {waddr[16:0], 6'b000000};    // write 256bytes
            2'b11: sys_addr = {sba_addr[24:8], 6'b000000}; // read 256bytes	
        endcase
    end

    SDRAM_16bit SDR
    (
        .sys_CLK(i_clk_sdram),				    // clock
        .sys_CMD(cntrl0_user_command_register),	// 00=nop, 01 = write 256 bytes, 10=read 32 bytes, 11=read 256 bytes
        .sys_ADDR(sys_addr),	                // word address
        .sys_DIN(cntrl0_user_input_data),		// data input
        .sys_DOUT(sys_DOUT),					// data output
        .sys_rd_data_valid(sys_rd_data_valid),	// data valid read
        .sys_wr_data_valid(sys_wr_data_valid),	// data valid write
        .sys_cmd_ack(sys_cmd_ack),			    // command acknowledged
        
        .sdr_n_CS_WE_RAS_CAS({SDRAM_CS, SDRAM_WE, SDRAM_RAS, SDRAM_CAS}),			// SDRAM #CS, #WE, #RAS, #CAS
        .sdr_BA(SDRAM_BA),					// SDRAM bank address
        .sdr_ADDR(SDRAM_ADDR),				// SDRAM address
        .sdr_DATA(SDRAM_D),	    			// SDRAM data
        .sdr_DQM(SDRAM_DQM)					// SDRAM DQM
    );

    wire ddr_rd;
    wire ddr_wr;

    cache_controller cache_ctrl 
    (
        // Interface with the CPU
        .addr(sba_addr[25:0]), 
        .dout(ram_dat_r), 
        .din(sba_dat_w), 
        .clk(i_clk),
        .mreq(ram_stb), 
        .wmask(sba_we),
        .ce(ce),

        // Interface with SDRAM
        .ddr_din(sys_DOUT), 
        .ddr_dout(cntrl0_user_input_data), 
        .ddr_clk(i_clk_sdram), 
        .ddr_rd(ddr_rd), 
        .ddr_wr(ddr_wr),
        .waddr(waddr),
        .cache_write_data(crw && sys_rd_data_valid), // read DDR, write to cache
        .cache_read_data(crw && sys_wr_data_valid),

        // Control
        .flush(1'b0),
        .clear(1'b0)
    );

    reg nop;
    always @(posedge i_clk_sdram) begin
        nop <= sys_cmd_ack == 2'b00;
        if (ddr_wr) cntrl0_user_command_register <= 2'b01;		// write 256 bytes cache
        else if(ddr_rd) cntrl0_user_command_register <= 2'b11;	// read 256 bytes cache
        else cntrl0_user_command_register <= 2'b00;
        
        if (nop) case (sys_cmd_ack)
            2'b01, 2'b11: crw <= 1'b1;	// cache read/write			
        endcase
    end

    assign ram_ack = 1'b1;

    assign SDRAM_CKE = 1'b1;
    assign SDRAM_CLK = ~i_clk_sdram;

`else // SDRAM

    // 32 MiB of BRAM
    
    reg [31:0] BRAM[32*1024*1024/4];
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

    assign ce = 1'b1;

`endif // SDRAM

    // External bus
    assign o_ext_addr  = sba_addr[15:0];
    assign o_ext_dat_w = sba_dat_w;
    assign o_ext_we    = sba_we;
    assign o_ext_stb   = addr_is_ext & sba_stb;

    // Timer
    wire [31:0] timer_dat_r;
    wire timer_ack;
    timer #(
        .FREQ_HZ(FREQ_HZ)
    ) timer(
        .i_clk(i_clk),
        .i_rst(i_rst),
        .i_addr(sba_addr[3:0]),
        .i_stb(addr_is_timer & sba_stb),
        .o_ack(timer_ack),
        .o_dat_r(timer_dat_r)
    );     

endmodule
