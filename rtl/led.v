
`default_nettype none

module led(
	input wire i_clk,
	input wire i_rst,
	input wire i_stb,
	input wire i_we,
	input wire [31:0] i_dat_w,
	output wire [31:0] o_dat_r,
	output wire o_ack,
	output wire [7:0] o_led
);

	reg [7:0] led;
	assign o_led = led;

	assign o_ack = i_stb;

	always @(posedge i_clk)
		if (i_rst) led <= 8'd0;
		else if (i_we && i_stb)
			led <= i_dat_w[7:0];

	assign o_dat_r = {24'd0, led};

endmodule
