// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

`default_nettype none

module cfg(
    input wire i_stb,
    output wire [31:0] o_dat_r,
    output wire o_ack
);

    assign o_ack = i_stb;
    assign o_dat_r = 32'd1;

endmodule
