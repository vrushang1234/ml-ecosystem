/* 
Reusing the same input register for storing the 
nagated value. 
Always returns the Two's complement verison of input
*/
`timescale 1ns / 1ps
`include "constants.h" 
module negation_module #(parameter N= `DATA_WIDTH )
    (
        input  logic [N-1:0] B,
        output logic [N-1:0] operand_B
    );
    always_comb begin 

        operand_B =(~B + 1'b1); 
    end 
    
endmodule
