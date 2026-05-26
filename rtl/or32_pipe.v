// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

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

module or32_pipe(
    input               i_rst,
    input               i_clk,
    input               i_ce,
    output reg [31:0]   o_addr,
    output reg [31:0]   o_dat_w,
    output reg [3:0]    o_we,
    input      [31:0]   i_dat_r,
    output reg          o_stb,
    input               i_ack
);

    reg [31:0] regs[16];

    // IF
    reg        if_wait;
    reg [31:0] if_pc;
    reg [31:0] if_instr_latch;
    reg        if_push;
    reg        if_pending;

    // ID
    reg        id_valid;
    reg [31:0] id_pc;
    reg [31:0] id_instr;
    reg [7:0]  id_arg1, id_arg2, id_arg3;
    reg [3:0]  id_rd;
    reg        id_reg_write;
    reg        id_mem_read;
    reg        id_mem_write;
    reg        id_is_div;
    reg        id_is_byte;

    // EX
    reg        ex_valid;
    reg [31:0] ex_pc;
    reg [31:0] ex_instr;
    reg [7:0]  ex_arg1, ex_arg2, ex_arg3;
    reg [3:0]  ex_rd;
    reg        ex_reg_write;
    reg        ex_mem_read;
    reg        ex_mem_write;
    reg        ex_is_byte;
    reg [31:0] ex_store_val;

    // MEM
    reg        mem_valid;
    reg [3:0]  mem_rd;
    reg        mem_mem_read;
    reg        mem_is_byte;
    reg [31:0] mem_addr;
    reg [31:0] mem_store_data;
    reg        mem_wait;
    reg [31:0] mem_load_data;

    // WB (forwarding pulse)
    reg        wb_valid;
    reg [3:0]  wb_rd;
    reg        wb_reg_write;
    reg [31:0] wb_wdata;

    // DIV
    reg        div_start;
    reg        div_busy;
    reg [3:0]  div_rd;
    wire       div_done;
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

    function automatic [31:0] read_arg;
        input [7:0]  arg;
        input [31:0] rf_val;
        begin
            if (arg[7:4] == 4'h8)
                read_arg = rf_val;
            else if (arg[7:4] < 4'h8)
                read_arg = {24'h0, arg};
            else
                read_arg = {{24{arg[7]}}, arg};
        end
    endfunction

    function automatic [31:0] fwd_mux;
        input [7:0]  arg;
        input [31:0] rf;
        input        ex_m, mem_m, wb_m;
        input [31:0] ex_v, mem_v, wb_v;
        begin
            if (arg[7:4] != 4'h8)
                fwd_mux = read_arg(arg, rf);
            else if (ex_m)
                fwd_mux = ex_v;
            else if (mem_m)
                fwd_mux = mem_v;
            else if (wb_m)
                fwd_mux = wb_v;
            else
                fwd_mux = rf;
        end
    endfunction

    wire stall_all     = div_busy;
    wire mem_port_busy = mem_valid;

    // ID register read (RIP uses PC+4)
    wire [31:0] rf_rs1 = (id_arg1[7:4] == 4'h8 && id_arg1[3:0] == `RIP) ? id_pc + 32'h4 :
                         (id_arg1[7:4] == 4'h8) ? regs[id_arg1[3:0]] : 32'd0;
    wire [31:0] rf_rs2 = (id_arg2[7:4] == 4'h8 && id_arg2[3:0] == `RIP) ? id_pc + 32'h4 :
                         (id_arg2[7:4] == 4'h8) ? regs[id_arg2[3:0]] : 32'd0;
    wire [31:0] rf_rs3 = (id_arg3[7:4] == 4'h8 && id_arg3[3:0] == `RIP) ? id_pc + 32'h4 :
                         (id_arg3[7:4] == 4'h8) ? regs[id_arg3[3:0]] : 32'd0;

    wire [31:0] ex_rf_rs1 = (ex_arg1[7:4] == 4'h8 && ex_arg1[3:0] == `RIP) ? ex_pc + 32'h4 :
                            (ex_arg1[7:4] == 4'h8) ? regs[ex_arg1[3:0]] : 32'd0;
    wire [31:0] ex_rf_rs2 = (ex_arg2[7:4] == 4'h8 && ex_arg2[3:0] == `RIP) ? ex_pc + 32'h4 :
                            (ex_arg2[7:4] == 4'h8) ? regs[ex_arg2[3:0]] : 32'd0;
    wire [31:0] ex_rf_rs3 = (ex_arg3[7:4] == 4'h8 && ex_arg3[3:0] == `RIP) ? ex_pc + 32'h4 :
                            (ex_arg3[7:4] == 4'h8) ? regs[ex_arg3[3:0]] : 32'd0;

    reg [31:0] mem_result_val;
    always @(*) begin
        mem_result_val = mem_load_data;
        if (mem_gpr_we) begin
            if (mem_is_byte) begin
                case (mem_addr[1:0])
                    2'b00: mem_result_val = {24'd0, i_dat_r[7:0]};
                    2'b01: mem_result_val = {24'd0, i_dat_r[15:8]};
                    2'b10: mem_result_val = {24'd0, i_dat_r[23:16]};
                    2'b11: mem_result_val = {24'd0, i_dat_r[31:24]};
                endcase
            end else
                mem_result_val = i_dat_r;
        end
    end

    wire [31:0] mem_fwd_val = mem_result_val;

    wire [31:0] ex_arg1_val = fwd_mux(ex_arg1, ex_rf_rs1, 1'b0, mem_ex_fwd_a, wb_ex_fwd_a,
                                      32'd0, mem_fwd_val, wb_wdata);
    wire [31:0] ex_arg2_val = fwd_mux(ex_arg2, ex_rf_rs2, 1'b0, mem_ex_fwd_b, wb_ex_fwd_b,
                                      32'd0, mem_fwd_val, wb_wdata);
    wire [31:0] ex_arg3_val = fwd_mux(ex_arg3, ex_rf_rs3, 1'b0, mem_ex_fwd_c, wb_ex_fwd_c,
                                      32'd0, mem_fwd_val, wb_wdata);

    wire [3:0]  ex_op = ex_instr[3:0];
    wire [31:0] ex_ims_base = (ex_arg1[7:4] == 4'h8) ? ex_arg1_val :
                              read_arg(ex_arg1, ex_rf_rs1);

    reg [31:0] ex_alu_val;
    always @(*) begin
        ex_alu_val = 32'd0;
        if (ex_valid && ex_instr[7:4] == 4'h7) begin
            case (ex_op)
                `OP_ADD:  ex_alu_val = ex_arg2_val + ex_arg3_val;
                `OP_SUB:  ex_alu_val = ex_arg2_val - ex_arg3_val;
                `OP_MUL:  ex_alu_val = ex_arg2_val * ex_arg3_val;
                `OP_AND:  ex_alu_val = ex_arg2_val & ex_arg3_val;
                `OP_OR:   ex_alu_val = ex_arg2_val | ex_arg3_val;
                `OP_SHL:  ex_alu_val = ex_arg2_val << ex_arg3_val[4:0];
                `OP_SHRU: ex_alu_val = ex_arg2_val >> ex_arg3_val[4:0];
                `OP_LTU:  ex_alu_val = ex_arg2_val < ex_arg3_val ? 32'd1 : 32'd0;
                `OP_IMS:  ex_alu_val = {ex_ims_base[15:0], ex_arg3, ex_arg2};
                default:  ex_alu_val = 32'd0;
            endcase
        end
    end

    wire [31:0] ex_mem_addr = ex_arg2_val + ex_arg3_val;

    wire ex_fwd = ex_valid && ex_reg_write && !ex_mem_read && !ex_mem_write;
    wire mem_fwd = mem_valid && mem_mem_read && (mem_gpr_we || !mem_wait);
    wire wb_fwd  = wb_valid && wb_reg_write;

    wire ex_fwd_a = ex_fwd && id_arg1[7:4] == 4'h8 && ex_rd == id_arg1[3:0];
    wire ex_fwd_b = ex_fwd && id_arg2[7:4] == 4'h8 && ex_rd == id_arg2[3:0];
    wire ex_fwd_c = ex_fwd && id_arg3[7:4] == 4'h8 && ex_rd == id_arg3[3:0];

    wire mem_fwd_a = mem_fwd && id_arg1[7:4] == 4'h8 && mem_rd == id_arg1[3:0];
    wire mem_fwd_b = mem_fwd && id_arg2[7:4] == 4'h8 && mem_rd == id_arg2[3:0];
    wire mem_fwd_c = mem_fwd && id_arg3[7:4] == 4'h8 && mem_rd == id_arg3[3:0];

    wire wb_fwd_a = wb_fwd && id_arg1[7:4] == 4'h8 && wb_rd == id_arg1[3:0];
    wire wb_fwd_b = wb_fwd && id_arg2[7:4] == 4'h8 && wb_rd == id_arg2[3:0];
    wire wb_fwd_c = wb_fwd && id_arg3[7:4] == 4'h8 && wb_rd == id_arg3[3:0];

    wire mem_ex_fwd = mem_valid && mem_mem_read && (mem_gpr_we || !mem_wait);
    wire wb_ex_fwd  = wb_valid && wb_reg_write;

    wire mem_ex_fwd_a = mem_ex_fwd && ex_arg1[7:4] == 4'h8 && mem_rd == ex_arg1[3:0];
    wire mem_ex_fwd_b = mem_ex_fwd && ex_arg2[7:4] == 4'h8 && mem_rd == ex_arg2[3:0];
    wire mem_ex_fwd_c = mem_ex_fwd && ex_arg3[7:4] == 4'h8 && mem_rd == ex_arg3[3:0];

    wire wb_ex_fwd_a = wb_ex_fwd && ex_arg1[7:4] == 4'h8 && wb_rd == ex_arg1[3:0];
    wire wb_ex_fwd_b = wb_ex_fwd && ex_arg2[7:4] == 4'h8 && wb_rd == ex_arg2[3:0];
    wire wb_ex_fwd_c = wb_ex_fwd && ex_arg3[7:4] == 4'h8 && wb_rd == ex_arg3[3:0];

    wire [31:0] id_arg1_val = fwd_mux(id_arg1, rf_rs1, ex_fwd_a, mem_fwd_a, wb_fwd_a,
                                      ex_alu_val, mem_fwd_val, wb_wdata);
    wire [31:0] id_arg2_val = fwd_mux(id_arg2, rf_rs2, ex_fwd_b, mem_fwd_b, wb_fwd_b,
                                      ex_alu_val, mem_fwd_val, wb_wdata);
    wire [31:0] id_arg3_val = fwd_mux(id_arg3, rf_rs3, ex_fwd_c, mem_fwd_c, wb_fwd_c,
                                      ex_alu_val, mem_fwd_val, wb_wdata);

    wire load_use = id_valid && (
        (mem_valid && mem_mem_read && (
            (id_arg1[7:4] == 4'h8 && id_arg1[3:0] == mem_rd) ||
            (id_arg2[7:4] == 4'h8 && id_arg2[3:0] == mem_rd) ||
            (id_arg3[7:4] == 4'h8 && id_arg3[3:0] == mem_rd))) ||
        (ex_valid && ex_mem_read && (
            (id_arg1[7:4] == 4'h8 && id_arg1[3:0] == ex_rd) ||
            (id_arg2[7:4] == 4'h8 && id_arg2[3:0] == ex_rd) ||
            (id_arg3[7:4] == 4'h8 && id_arg3[3:0] == ex_rd)))
    );

    wire load_use_ex = ex_valid && mem_valid && mem_mem_read && (
        (ex_arg1[7:4] == 4'h8 && ex_arg1[3:0] == mem_rd) ||
        (ex_arg2[7:4] == 4'h8 && ex_arg2[3:0] == mem_rd) ||
        (ex_arg3[7:4] == 4'h8 && ex_arg3[3:0] == mem_rd)
    );

    wire ex_branch_taken = ex_valid && ex_instr[7:4] == 4'h7 &&
                           ex_instr[3:0] == `OP_JZ && (ex_arg1_val == 32'd0);
    wire [31:0] branch_offset = {{14{ex_arg3[7]}}, ex_arg3, ex_arg2, 2'b00};

    wire advance = i_ce && !stall_all;

    wire mem_gpr_we = i_ce && mem_valid && mem_wait && i_ack && mem_mem_read;
    wire rip_mem_wb = mem_gpr_we && mem_rd == `RIP;

    wire ex_gpr_we = i_ce && ex_valid && ex_reg_write &&
                     !ex_mem_read && !ex_mem_write && !ex_branch_taken && !load_use_ex &&
                     !ld_rip_wb;
    wire div_gpr_we = i_ce && div_busy && div_done;

    wire ex_rip_alu  = ex_gpr_we && ex_rd == `RIP;
    wire ld_rip_wb   = mem_gpr_we && mem_rd == `RIP;
    wire flush_pipe  = ex_branch_taken || ex_rip_alu || ld_rip_wb;

    wire ex_mem_issue = ex_valid && (ex_mem_read || ex_mem_write) && !flush_pipe;
    wire ex_noop_done = ex_valid && ex_instr[7:4] == 4'h7 &&
                        (ex_instr[3:0] == `OP_JZ || ex_instr[3:0] == `OP_SYS);
    wire ex_bad_op    = ex_valid && ex_instr[7:4] != 4'h7;
    wire ex_alu_done  = ex_valid && ex_reg_write && !ex_mem_read && !ex_mem_write &&
                        !load_use_ex;

    wire mem_busy_stall = id_valid && (id_mem_read || id_mem_write) && mem_valid;
    wire stall_pipe     = load_use || load_use_ex || mem_busy_stall;

    wire ex_busy      = ex_valid;
    wire ex_can_retire = ex_branch_taken || ex_noop_done || ex_alu_done || ex_bad_op;

    wire stall_ID_pre = stall_all || stall_pipe;
    wire mem_new      = !stall_ID_pre && !load_use && !load_use_ex &&
                        !mem_busy_stall && ex_mem_issue;
    wire ex_retire    = ex_can_retire || mem_new;
    wire block_id_issue = ex_mem_issue && mem_new;

    wire id_can_issue = id_valid && !flush_pipe && !ex_branch_taken &&
                        (!ex_busy || ex_retire);
    wire stall_ID    = stall_ID_pre || (id_valid && !id_can_issue);
    wire id_issue     = !stall_ID && id_can_issue && !block_id_issue && !flush_pipe;
    wire if_id_ready  = !id_valid || id_issue;

    wire rip_in_flight = (id_valid && id_reg_write && id_rd == `RIP) ||
                         (ex_valid && ex_reg_write && ex_rd == `RIP) ||
                         (mem_valid && mem_mem_read && mem_rd == `RIP);

    wire stall_fetch = stall_all || mem_port_busy || stall_pipe ||
                       (id_valid && !id_can_issue) || if_pending || rip_in_flight ||
                       id_valid || ex_valid || mem_valid || div_busy;

    // Pulse o_stb only when starting a mem access (FSM-style); wait for ack with stb low.
    wire mem_req  = mem_new;
    wire if_start = !if_wait && !stall_fetch;
    wire if_req   = if_start && !mem_req;

    wire        bus_mem_read    = mem_new ? ex_mem_read : mem_mem_read;
    wire        bus_mem_is_byte = mem_new ? ex_is_byte : mem_is_byte;
    wire [31:0] bus_mem_addr    = mem_new ? ex_mem_addr : mem_addr;
    wire [31:0] bus_store_data  = mem_new ? ex_store_val : mem_store_data;

    always @(posedge i_clk) begin
        div_start <= 1'b0;

        if (i_rst) begin
            regs[`RPP] <= 32'h0;
            regs[`RIP] <= 32'h0;
            o_we         <= 4'h0;
            o_stb        <= 1'b0;
            if_wait      <= 1'b0;
            if_push      <= 1'b0;
            if_pending   <= 1'b0;
            id_valid     <= 1'b0;
            ex_valid     <= 1'b0;
            mem_valid    <= 1'b0;
            wb_valid     <= 1'b0;
            mem_wait     <= 1'b0;
            div_busy     <= 1'b0;
        end else if (i_ce) begin
            if (advance) begin
                o_stb <= 1'b0;
                o_we  <= 4'h0;
            end

            if (advance && mem_req) begin
                o_addr <= {bus_mem_addr[31:2], 2'b0};
                o_stb  <= 1'b1;
                if (!bus_mem_read) begin
                    if (bus_mem_is_byte) begin
                        case (bus_mem_addr[1:0])
                            2'b00: begin o_dat_w <= {24'd0, bus_store_data[7:0]}; o_we <= 4'b0001; end
                            2'b01: begin o_dat_w <= {16'd0, bus_store_data[7:0], 8'd0}; o_we <= 4'b0010; end
                            2'b10: begin o_dat_w <= {8'd0, bus_store_data[7:0], 16'd0}; o_we <= 4'b0100; end
                            2'b11: begin o_dat_w <= {bus_store_data[7:0], 24'd0}; o_we <= 4'b1000; end
                        endcase
                    end else begin
                        o_dat_w <= bus_store_data;
                        o_we    <= 4'b1111;
                    end
                end
            end else if (advance && if_req) begin
                o_addr <= regs[`RIP];
                o_stb  <= 1'b1;
                o_we   <= 4'h0;
            end

            if (ex_gpr_we)
                regs[ex_rd] <= ex_alu_val;
            else if (mem_gpr_we) begin
                if (mem_is_byte) begin
                    case (mem_addr[1:0])
                        2'b00: regs[mem_rd] <= {24'd0, i_dat_r[7:0]};
                        2'b01: regs[mem_rd] <= {24'd0, i_dat_r[15:8]};
                        2'b10: regs[mem_rd] <= {24'd0, i_dat_r[23:16]};
                        2'b11: regs[mem_rd] <= {24'd0, i_dat_r[31:24]};
                    endcase
                end else
                    regs[mem_rd] <= i_dat_r;
            end else if (div_gpr_we)
                regs[div_rd] <= div_val;

            if (ex_branch_taken && !rip_mem_wb)
                regs[`RIP] <= ex_pc + 32'h4 + branch_offset;

            wb_valid     <= 1'b0;
            wb_reg_write <= 1'b0;
            if (ex_gpr_we) begin
                wb_valid     <= 1'b1;
                wb_rd        <= ex_rd;
                wb_reg_write <= 1'b1;
                wb_wdata     <= ex_alu_val;
            end else if (mem_gpr_we) begin
                wb_valid     <= 1'b1;
                wb_rd        <= mem_rd;
                wb_reg_write <= 1'b1;
                wb_wdata     <= mem_result_val;
            end else if (div_gpr_we) begin
                wb_valid     <= 1'b1;
                wb_rd        <= div_rd;
                wb_reg_write <= 1'b1;
                wb_wdata     <= div_val;
            end

            if (mem_valid && mem_wait && i_ack) begin
                mem_wait  <= 1'b0;
                mem_valid <= 1'b0;
                if (mem_mem_read)
                    mem_load_data <= i_dat_r;
            end

            if (flush_pipe) begin
                if_wait <= 1'b0;
            end else if (if_start) begin
                if_pc   <= regs[`RIP];
                if_wait <= 1'b1;
                if (!ex_branch_taken && !(ex_gpr_we && ex_rd == `RIP) && !rip_mem_wb)
                    regs[`RIP] <= regs[`RIP] + 32'h4;
            end else if (if_wait && i_ack) begin
                if_wait        <= 1'b0;
                if_instr_latch <= i_dat_r;
                if (if_id_ready)
                    if_push <= 1'b1;
                else
                    if_pending <= 1'b1;
            end else if (!stall_ID && if_pending && if_id_ready) begin
                if_push    <= 1'b1;
                if_pending <= 1'b0;
            end

            if (advance) begin

                if (mem_new && ex_mem_read) begin
                    mem_valid      <= 1'b1;
                    mem_rd         <= ex_rd;
                    mem_mem_read   <= 1'b1;
                    mem_is_byte    <= ex_is_byte;
                    mem_addr       <= ex_mem_addr;
                    mem_store_data <= 32'd0;
                    mem_wait       <= 1'b1;
                end else if (mem_new && ex_mem_write) begin
                    mem_valid      <= 1'b1;
                    mem_rd         <= 4'h0;
                    mem_mem_read   <= 1'b0;
                    mem_is_byte    <= ex_is_byte;
                    mem_addr       <= ex_mem_addr;
                    mem_store_data <= ex_store_val;
                    mem_wait       <= 1'b1;
                end

                if (flush_pipe) begin
                    id_valid       <= 1'b0;
                    ex_valid       <= 1'b0;
                    if (!(mem_valid && mem_wait))
                        mem_valid  <= 1'b0;
                    if_push        <= 1'b0;
                    if_pending     <= 1'b0;
                    if_instr_latch <= 32'd0;
                    ex_instr       <= 32'd0;
                    ex_reg_write   <= 1'b0;
                    ex_mem_read    <= 1'b0;
                    ex_mem_write   <= 1'b0;
                end else begin
                    if (id_issue) begin
                        if (id_is_div) begin
                            div_a     <= id_arg2_val;
                            div_b     <= id_arg3_val;
                            div_rd    <= id_rd;
                            div_start <= 1'b1;
                            div_busy  <= 1'b1;
                        end else begin
                            ex_valid      <= 1'b1;
                            ex_pc         <= id_pc;
                            ex_instr      <= id_instr;
                            ex_arg1       <= id_arg1;
                            ex_arg2       <= id_arg2;
                            ex_arg3       <= id_arg3;
                            ex_rd         <= id_rd;
                            ex_reg_write  <= id_reg_write;
                            ex_mem_read   <= id_mem_read;
                            ex_mem_write  <= id_mem_write;
                            ex_is_byte    <= id_is_byte;
                            ex_store_val  <= id_arg1_val;
                        end
                    end else if (ex_retire)
                        ex_valid <= 1'b0;
                end

                if (flush_pipe) begin
                    if_push <= 1'b0;
                end else if (!stall_ID) begin
                    if (if_push) begin
                        id_valid      <= 1'b1;
                        id_pc         <= if_pc;
                        id_instr      <= if_instr_latch;
                        id_arg1       <= if_instr_latch[15:8];
                        id_arg2       <= if_instr_latch[23:16];
                        id_arg3       <= if_instr_latch[31:24];
                        id_rd         <= if_instr_latch[11:8];
                        id_reg_write  <= 1'b0;
                        id_mem_read   <= 1'b0;
                        id_mem_write  <= 1'b0;
                        id_is_div     <= 1'b0;
                        id_is_byte    <= 1'b0;
                        if (if_instr_latch[7:4] == 4'h7) begin
                            case (if_instr_latch[3:0])
                                `OP_ADD, `OP_SUB, `OP_MUL, `OP_AND, `OP_OR,
                                `OP_SHL, `OP_SHRU, `OP_LTU, `OP_IMS:
                                    id_reg_write <= 1'b1;
                                `OP_DIV:
                                    id_is_div <= 1'b1;
                                `OP_LDW: begin
                                    id_reg_write <= 1'b1;
                                    id_mem_read  <= 1'b1;
                                end
                                `OP_LDB: begin
                                    id_reg_write <= 1'b1;
                                    id_mem_read  <= 1'b1;
                                    id_is_byte   <= 1'b1;
                                end
                                `OP_STW:
                                    id_mem_write <= 1'b1;
                                `OP_STB: begin
                                    id_mem_write <= 1'b1;
                                    id_is_byte   <= 1'b1;
                                end
                                default: ;
                            endcase
                        end
                        if_push <= 1'b0;
                    end else if (id_issue)
                        id_valid <= 1'b0;
                end
            end
            if (div_busy && div_done)
                div_busy <= 1'b0;
        end
    end

endmodule
