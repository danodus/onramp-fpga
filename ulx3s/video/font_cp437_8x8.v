// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// PC font (code page 437)

`default_nettype none

module font_cp437_8x8(
    input [10:0] addr,
    output [7:0] data
);

    assign data = bitarray[addr];

    reg [7:0] bitarray[0:2047];
    
    initial begin
        $readmemh("font_cp437_8x8.hex", bitarray);
    end

endmodule
