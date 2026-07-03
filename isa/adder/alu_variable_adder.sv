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
    logic [N-1:0] adder_b; //wire used to be either the original B input for addtion,
                           //or negated B for subtraction 
    negation_module #(.N(N)) neg_inst (
        .B(B),
        .operand_B(operand_B)
    );
    assign adder_b = subtract ? operand_B : B;
    assign Sum = A + adder_b;
    assign overflow = (A[N-1] == adder_b[N-1]) && (Sum[N-1] != A[N-1]);
endmodule
