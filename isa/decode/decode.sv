`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 07/03/2026 10:09:59 PM
// Design Name: 
// Module Name: decode
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module decode (
    
    input logic [11:0] instruction,
    output logic [3:0] opcode,
    output logic [1:0] engine,
    output logic [1:0] op1,
    output logic [1:0] op2,
    output logic out
    );
 
    always_comb begin
        opcode = instruction[7:4];
        //buffer assignment
        op1 = instruction[3:2];
        op2 = instruction[1:0];
         
        case (opcode)
        //engine routing
            4'b0000: engine = 2'b10; //add
            4'b0001: engine = 2'b10; //sub
            4'b0010: engine = 2'b10; //MatMul
            4'b0011: engine = 2'b10; //addT
            4'b0100: engine = 2'b10; //subT
            4'b0101: engine = 2'b10; //loadT
            default: engine = 2'b00; //default
        endcase
    end
   

endmodule

