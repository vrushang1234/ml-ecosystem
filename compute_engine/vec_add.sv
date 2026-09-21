`timescale 1ns / 1ps

// Elementwise vector addition: Sum[i] = A[i] + B[i] for i in [0, M).
module vec_add #(
    parameter int WIDTH = 8,
    parameter int M     = 4
) (
    input  logic signed [WIDTH-1:0] A   [M],
    input  logic signed [WIDTH-1:0] B   [M],
    output logic signed [WIDTH-1:0] Sum [M],
    output logic overflow [M]
);
    always_comb begin
        for (int i = 0; i < M; i++) begin
            Sum[i] = A[i] + B[i];
            // Signed overflow: only possible when both operands share a
            // sign and the result's sign doesn't match theirs 
            overflow[i] = (A[i][WIDTH-1] == B[i][WIDTH-1]) &&
                          (Sum[i][WIDTH-1] != A[i][WIDTH-1]);
        end
    end
endmodule