// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Registers
`define RPP 4'hE
`define RIP 4'hF

// Opcodes
`define OP_ADD  4'h0
`define OP_SUB  4'h1
`define OP_MUL  4'h2
`define OP_DIV  4'h3
`define OP_AND  4'h4
`define OP_OR   4'h5
`define OP_SHL  4'h6
`define OP_SHRU 4'h7
`define OP_LDW  4'h8
`define OP_STW  4'h9
`define OP_LDB  4'hA
`define OP_STB  4'hB
`define OP_IMS  4'hC
`define OP_LTU  4'hD
`define OP_JZ   4'hE
`define OP_SYS  4'hF

module or32(
    input               i_rst,
    input               i_clk,
    output reg [31:0]   o_addr,
    output reg [31:0]   o_dat_w,
    output reg [3:0]    o_we,
    input      [31:0]   i_dat_r,
	output reg          o_stb,
	input               i_ack
);


    localparam FETCH      = 4'd0;
    localparam FETCH_WAIT = 4'd1;
    localparam EXECUTE    = 4'd2;
    localparam LOAD       = 4'd3;
    localparam LOAD_WAIT  = 4'd4;
    localparam STORE      = 4'd5;
    localparam STORE_WAIT = 4'd6;
    localparam DIV_WAIT   = 4'd7;

    reg [31:0] regs[16];

    reg [3:0] state;

    reg [31:0] instr;
    wire [7:0] opcode = instr[7:0];
    wire [7:0] arg1 = instr[15:8];
    wire [7:0] arg2 = instr[23:16];
    wire [7:0] arg3 = instr[31:24];

    wire [31:0] arg1_val = (arg1[7:4] == 4'h8) ? regs[arg1[3:0]] : (arg1[7:4] < 4'h8) ? {24'h0, arg1} : {24'hFFFFFF, arg1}; 
    wire [31:0] arg2_val = (arg2[7:4] == 4'h8) ? regs[arg2[3:0]] : (arg2[7:4] < 4'h8) ? {24'h0, arg2} : {24'hFFFFFF, arg2}; 
    wire [31:0] arg3_val = (arg3[7:4] == 4'h8) ? regs[arg3[3:0]] : (arg3[7:4] < 4'h8) ? {24'h0, arg3} : {24'hFFFFFF, arg3}; 

    wire [31:0] next_ip = regs[`RIP] + 32'h4;

    wire [31:0] addr = arg2_val + arg3_val;

    // Division
    reg div_start;
    wire div_done;
    reg [31:0] div_a, div_b;
    wire [31:0] div_val;
    
    div div(
        .i_rst(i_rst),
        .i_clk(i_clk),
        .i_start(div_start),
        .o_busy(),
        .o_done(div_done),
        .o_valid(),
        .o_dbz(),
        .i_a(div_a),
        .i_b(div_b),
        .o_val(div_val),
        .o_rem()
    );

    always @(posedge i_clk) begin
        if (i_rst) begin
            state <= FETCH;
            o_we <= 4'h0;
            o_stb <= 1'b0;
            regs[`RPP] <= 32'h00000000;
            regs[`RIP] <= 32'h00000000;
            div_start <= 1'b0;
        end else begin
            case (state)
                FETCH: begin
                    o_addr <= regs[`RIP];
                    regs[`RIP] <= next_ip;
                    o_stb <= 1'b1;
                    state <= FETCH_WAIT;
                end
                FETCH_WAIT: begin
                    o_stb <= 1'b0;
                    if (i_ack) begin
                        instr <= i_dat_r;
                        state <= EXECUTE;
                    end
                end
                EXECUTE: begin
                    state <= FETCH;
                    if (opcode[7:4] == 4'h7) begin
                        case (opcode[3:0])
                            `OP_ADD:
                                regs[arg1[3:0]] <= arg2_val + arg3_val;
                            `OP_SUB:
                                regs[arg1[3:0]] <= arg2_val - arg3_val;
                            `OP_MUL:
                                regs[arg1[3:0]] <= arg2_val * arg3_val;
                            `OP_DIV: begin
                                div_a <= arg2_val;
                                div_b <= arg3_val;
                                div_start <= 1'b1;
                                state <= DIV_WAIT;
                            end
                            `OP_AND:
                                regs[arg1[3:0]] <= arg2_val & arg3_val;
                            `OP_OR:
                                regs[arg1[3:0]] <= arg2_val | arg3_val;
                            `OP_SHL:
                                regs[arg1[3:0]] <= arg2_val << arg3_val;
                            `OP_SHRU:
                                regs[arg1[3:0]] <= arg2_val >> arg3_val;
                            `OP_LDW:
                                state <= LOAD;
                            `OP_LDB:
                                state <= LOAD;
                            `OP_STW:
                                state <= STORE;
                            `OP_STB:
                                state <= STORE;
                            `OP_IMS:
                                regs[arg1[3:0]] <= {regs[arg1[3:0]][15:0], arg3, arg2}; 
                            `OP_LTU:
                                regs[arg1[3:0]] <= arg2_val < arg3_val ? 32'd1 : 32'd0;
                            `OP_JZ:
                                if (arg1_val == 32'd0)
                                    regs[`RIP] <= regs[`RIP] + {{{14{arg3[7]}}, arg3, arg2}, 2'b00};
                            `OP_SYS: begin
                            end
                        endcase
                    end
                end
                LOAD: begin
                    o_addr <= {addr[31:2], 2'b0};
                    o_stb <= 1'b1;
                    state <= LOAD_WAIT;
                end
                LOAD_WAIT: begin
                    o_stb <= 1'b0;
                    if (i_ack) begin
                        if (opcode[3:0] == `OP_LDB) begin
                            case (addr[1:0])
                                2'b00: regs[arg1[3:0]] <= {24'd0, i_dat_r[7:0]};
                                2'b01: regs[arg1[3:0]] <= {24'd0, i_dat_r[15:8]};
                                2'b10: regs[arg1[3:0]] <= {24'd0, i_dat_r[23:16]};
                                2'b11: regs[arg1[3:0]] <= {24'd0, i_dat_r[31:24]};
                            endcase
                        end else begin
                            regs[arg1[3:0]] <= i_dat_r;
                        end
                        state <= FETCH;
                    end
                end
                STORE: begin
                    o_addr <= {addr[31:2], 2'b0};
                    if (opcode[3:0] == `OP_STB) begin
                        case (addr[1:0])
                            2'b00: begin
                                o_dat_w <= {24'd0, arg1_val[7:0]};
                                o_we <= 4'b0001;
                            end
                            2'b01: begin
                                o_dat_w <= {16'd0, arg1_val[7:0], 8'd0};
                                o_we <= 4'b0010;
                            end
                            2'b10: begin
                                o_dat_w <= {8'd0, arg1_val[7:0], 16'd0};
                                o_we <= 4'b0100;
                            end
                            2'b11: begin
                                o_dat_w <= {arg1_val[7:0], 24'd0};
                                o_we <= 4'b1000;
                            end
                        endcase
                    end else begin
                        o_dat_w <= arg1_val;
                        o_we <= 4'b1111;
                    end
                    o_stb <= 1'b1;
                    state <= STORE_WAIT;
                end
                STORE_WAIT: begin
                    o_stb <= 1'b0;
                    if (i_ack) begin
                        o_we <= 4'h0;
                        state <= FETCH;
                    end
                end
                DIV_WAIT: begin
                    div_start <= 1'b0;
                    if (div_done) begin
                        regs[arg1[3:0]] <= div_val;
                        state <= FETCH;
                    end
                end
                default: begin
                    state <= FETCH;
                end
            endcase
        end
    end

endmodule
