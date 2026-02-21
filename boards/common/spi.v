// Copyright (c) 2025-2026 Daniel Cliche
// SPDX-License-Identifier: MIT

// Ref.: https://gitlab.com/x653/xv6-riscv-fpga
//       (c) 2023 Michael Schröder
// Ref.: Project Oberon (https://www.projectoberon.net/)
//       (c) 2013 Niklaus Wirth (NW), Juerg Gutknecht (JG), Paul Reed (PR/PDR)

`default_nettype none

module spi #(
    parameter FREQ_HZ = 25_000_000
) (
    input wire i_clk,
    input wire i_rst,
    input wire i_stb,
    input wire i_we,
    input wire [31:0] i_dat_w,
    input wire [3:0] i_addr,
    output wire [31:0] o_dat_r,
    output reg o_ack,
    output reg o_ss,
    output wire o_mosi,
    input wire i_miso,
    output wire o_sck
);

    wire start;
    reg fast;
    wire end_bit, end_tick;
    reg [31:0] shreg;
    localparam max_tick_slow = FREQ_HZ * 64 / 25_000_000 - 1;
    localparam max_tick_fast = FREQ_HZ * 3 / 25_000_000 - 1;
    reg [$clog2(max_tick_slow)-1:0] tick;
    reg [4:0] bit_cnt;
    reg rdy;

    assign end_tick = fast ? (tick == max_tick_fast) : (tick == max_tick_slow);
    assign end_bit = fast ? (bit_cnt == 31) : (bit_cnt == 7);
    assign o_dat_r = i_addr[2] ? {30'd0, fast, o_ss} : fast ? shreg : {24'b0, shreg[7:0]};
    assign o_mosi = (i_rst | rdy) ? 1 : shreg[7];
    assign o_sck = (i_rst | rdy) ? 0 : fast ? end_tick : tick[$clog2(max_tick_slow)-1];

    assign start = ~i_addr[2] & i_we & i_stb;

    always @(posedge i_clk) begin
        tick <= (i_rst | rdy | end_tick) ? 0 : tick + 1;
        rdy <= (i_rst | end_tick & end_bit) ? 1 : start ? 0 : rdy;
        bit_cnt <= (i_rst | start) ? 0 : (end_tick & ~end_bit) ? bit_cnt + 1 : bit_cnt;
        shreg <= i_rst ? -1 : start ? i_dat_w : end_tick ?
            {shreg[30:24], i_miso, shreg[22:16], shreg[31], shreg[14:8],
                   shreg[23], shreg[6:0], (fast ? shreg[15] : i_miso)} : shreg;
    end

    always @(posedge i_clk)
        if (i_rst) begin
            o_ss <= 1;
            fast <= 0;
        end	else if (i_stb & i_addr[2] & i_we) begin
            o_ss <= i_dat_w[0];
            fast <= i_dat_w[1];
        end
    
    always @(posedge i_clk)
        if (i_rst) o_ack <= 0;
        else if ((end_tick & end_bit) | (i_stb & ((~i_addr[2] & ~i_we)|i_addr[2] ))) o_ack <= 1;	
        else o_ack <= 0;

endmodule
