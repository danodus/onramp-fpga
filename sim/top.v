module top (
    input        clk,
    input        reset_i,

    output [7:0] display_o,

    input        rx_i,
    output       tx_o,

    output       vga_hsync,
    output       vga_vsync,
    output [7:0] vga_r,
    output [7:0] vga_g,
    output [7:0] vga_b,
    output       vga_de,

    input  [7:0]  ps2_kbd_code_i,
    input         ps2_kbd_strobe_i,
    input         ps2_kbd_err_i,

    // SDRAM
    output        sdram_clk_o,
    output        sdram_cke_o,
    output        sdram_cs_n_o,
    output        sdram_we_n_o,
    output        sdram_ras_n_o,
    output        sdram_cas_n_o,
    output [12:0] sdram_a_o,
    output [1:0]  sdram_ba_o,
    output [1:0]  sdram_dqm_o,
    inout  [15:0] sdram_dq_io  
    );

    soc SOC(
        .i_clk(clk),
        .i_rst(reset_i),
        .o_led(display_o)
    );

    // initial begin
    //     $display("[%0t] Tracing to logs/vlt_dump.vcd...\n", $time);
    //     $dumpfile("logs/vlt_dump.vcd");
    //     $dumpvars();
    // end

    // always_ff @(posedge clk) begin
    //     if ($time > 100000)
    //             $finish;
    // end

endmodule