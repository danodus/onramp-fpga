// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

module top (
    input  logic        clk,
    input  logic        i_rst,
    output logic [15:0] o_ext_addr,
    output logic        o_ext_stb,
    output logic [3:0]  o_ext_we,
    input  logic        i_ext_ack,
    output logic [31:0] o_ext_dat_w,
    input  logic [31:0] i_ext_dat_r
);

    soc soc(
        .i_clk(clk),
        .i_rst(i_rst),
        // External bus
        .o_ext_addr(o_ext_addr),
        .o_ext_stb(o_ext_stb),
        .o_ext_we(o_ext_we),
        .i_ext_ack(i_ext_ack),
        .o_ext_dat_w(o_ext_dat_w),
        .i_ext_dat_r(i_ext_dat_r)
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
