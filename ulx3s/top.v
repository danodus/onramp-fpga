module top(
   input clk,
   input [6:0] btn,
   output [7:0] led
);

    //reset
    reg [24:0]	rst_cnt = 0;
    wire		rst = btn[1] || !(& rst_cnt);
    always @(posedge clk) begin
        rst_cnt <= rst_cnt + {23'd0,rst};
    end

    //SOC
    soc soc(
        .i_clk(clk),
        .i_rst(rst),
        .o_led(led)
    );
   
endmodule
