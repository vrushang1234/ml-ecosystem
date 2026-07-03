`timescale 1ns / 1ps

`include "constants.h"

`timescale 1ns / 1ps
`include "constants.h"

module alu_variable_adder #(parameter N = `DATA_WIDTH) (
    input  logic [N-1:0] A,
    input  logic [N-1:0] B,
    input  logic  subtract, //right now input for this wire is coming from the test_bench
    output logic [N-1:0] Sum,
    output logic  overflow
);

    logic [N-1:0] operand_B;

    negation_module #(.N(N)) neg_inst (
        .B(B),
        .subtract(subtract),
        .operand_B(operand_B)
    );

    assign Sum = A + operand_B;
    assign overflow = (A[N-1] == operand_B[N-1]) && (Sum[N-1] != A[N-1]);

endmodule
