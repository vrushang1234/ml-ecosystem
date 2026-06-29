`timescale 1ns / 1ps

`include "constants.h"

//parameterized module #
module alu_variable_adder #(parameter N= `DATA_WIDTH )
        (
        input [N-1:0] A, 
        input [N-1:0] B, 
        input subtract, //1 for subtract, 0 for add 
        output [N-1:0] Sum, 
        output  overflow
    );
    //operand_B is the required +b or the -B depending on wherter your adding 
    // (A+B) or subtracting (A+(-B)) 
    wire [N-1:0] operand_B = subtract ? (~B + 1'b1) : B ; 
    
    assign Sum = A + operand_B; 
    //Overflow when A, B have the same sign, and Sum has a different sign 
    assign overflow = (  A[N-1] == operand_B[N-1] ) && (Sum[N-1]!=A[N-1]) ; 
    
    
endmodule
