/* 
Reusing the same input register for storing the 
nagated value. 
*/
`timescale 1ns / 1ps
`include "constants.h" 

module negation_module #(parameter N= `DATA_WIDTH )
    (
        input  logic [N-1:0] B,
        input  logic subtract,
        output logic [N-1:0] operand_B
    );
    always_comb begin 
        //if subtract HIGH, return two's complement of B
        //else return B as is
        operand_B = subtract ? (~B + 1'b1) : B ; 
    end 
    
endmodule
