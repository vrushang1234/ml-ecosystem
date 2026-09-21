`timescale 1ns / 1ps

// Elementwise ReLU: out[i] = (in[i] < 0) ? 0 : in[i], for i in [0, M).
// Similar set-up as vec_zero, there is an array of M signed WIDTH-bit numbers 
// being computed parrallely.
module relu_unit #(
    parameter int WIDTH = 8,
    parameter int M     = 4
) (
    input  logic signed [WIDTH-1:0] in  [M],
    output logic signed [WIDTH-1:0] out [M]
);
    always_comb begin
        for (int i = 0; i < M; i++) begin
            // Sign bit check, if negative set 0, 
            //else return value itself 
            out[i] = in[i][WIDTH-1] ? '0 : in[i];
        end
    end
endmodule