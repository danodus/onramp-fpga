// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

module top(
   input        clk,
   input  [6:0] btn,
   output [7:0] led
);

    // Reset
    reg [7:0]	rst_cnt = 0;
    wire		rst = btn[1] || !(& rst_cnt);
    always @(posedge clk) begin
        rst_cnt <= rst_cnt + {6'd0,rst};
    end

    // External SBA bus
    wire [15:0] ext_addr;
    wire        ext_stb;
    wire [3:0]  ext_we;
    wire        ext_ack;
    wire [31:0] ext_dat_w, ext_dat_r;

    // SoC
    soc soc(
        .i_clk(clk),
        .i_rst(rst),
        // External bus
        .o_ext_addr(ext_addr),
        .o_ext_stb(ext_stb),
        .o_ext_we(ext_we),
        .i_ext_ack(ext_ack),
        .o_ext_dat_w(ext_dat_w),
        .i_ext_dat_r(ext_dat_r)
    );

    // LED
    // Currently connected directly to the external bus.
    wire [31:0] led_dat_r;
    wire led_ack;
    led LED(
        .i_clk(clk),
        .i_rst(rst),
        .i_stb(ext_stb),
        .i_we(ext_we[0]),
        .o_ack(ext_ack),
        .i_dat_w(ext_dat_w),
        .o_dat_r(ext_dat_r),	
        .o_led(led)
    );

   
endmodule
