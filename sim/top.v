// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module top (
    input  logic        clk,
    input  logic        i_rst,
    output logic [27:0] o_ext_addr,
    output logic        o_ext_stb,
    output logic [3:0]  o_ext_we,
    input  logic        i_ext_ack,
    output logic [31:0] o_ext_dat_w,
    input  logic [31:0] i_ext_dat_r,
    // SDRAM
    output      logic        sdram_clk_o,
    output      logic        sdram_cke_o,
    output      logic        sdram_cs_n_o,
    output      logic        sdram_we_n_o,
    output      logic        sdram_ras_n_o,
    output      logic        sdram_cas_n_o,
    output      logic [12:0] sdram_a_o,
    output      logic [1:0]  sdram_ba_o,
    output      logic [1:0]  sdram_dqm_o,
    inout       logic [15:0] sdram_dq_io     
);

    soc soc(
        .i_clk(clk),
        .i_clk_sdram(clk),
        .i_rst(i_rst),
        // External bus
        .o_ext_addr(o_ext_addr),
        .o_ext_stb(o_ext_stb),
        .o_ext_we(o_ext_we),
        .i_ext_ack(i_ext_ack),
        .o_ext_dat_w(o_ext_dat_w),
        .i_ext_dat_r(i_ext_dat_r),
        // SDRAM
        .SDRAM_CLK(),                 // Clock for SDRAM chip
        .SDRAM_CKE(),                 // Clock enabled
        .SDRAM_D(sdram_dq_io),        // Bidirectional data lines to/from SDRAM
        .SDRAM_ADDR(sdram_a_o),       // Address bus, multiplexed, 13 bits
        .SDRAM_BA(sdram_ba_o),        // Bank select wires for 4 banks
        .SDRAM_DQM(sdram_dqm_o),      // Byte mask
        .SDRAM_CS(sdram_cs_n_o),      // Chip select
        .SDRAM_WE(sdram_we_n_o),      // Write enable
        .SDRAM_RAS(sdram_ras_n_o),    // Row address select
        .SDRAM_CAS(sdram_cas_n_o)     // Columns address select        
    );

    // initial begin
    //     $display("[%0t] Tracing to logs/vlt_dump.vcd...\n", $time);
    //     $dumpfile("logs/vlt_dump.vcd");
    //     $dumpvars();
    // end

    // always_ff @(posedge clk) begin
    //     if ($time > 10000)
    //             $finish;
    // end

endmodule
