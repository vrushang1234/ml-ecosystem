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
    
    input logic [31:0] instruction,
    output logic [3:0] opcode,
    output logic [1:0] engine,
    
    //for non store operations
    output logic [1:0] op1,
    output logic [1:0] op2,
    
    //for store operation -> opcode == 4'b0110
    output logic [1:0] scratch,
    output logic [1:0] offset, 
    output logic [1:0] row,
    output logic [1:0] col,
    output logic [3:0] stride, 
    output logic [1:0] acc
    );
 
    always_latch begin
        opcode = instruction[31:28];
        
        //store 
        if (opcode == 4'b0110) begin
            scratch = instruction[13:12];
            offset = instruction[11:10];
            row = instruction[9:8];
            col = instruction[7:6];
            stride = instruction[5:2];
            acc = instruction[1:0];
        end else begin
            scratch = 0;
            offset = 0;
            row = 0;
            col = 0;
            stride = 0;
            acc = 0;
            op1 = instruction[3:2];
            op2 = instruction[1:0];
            
        end
        case (opcode)
        //engine routing
        
        //ALU engine
            4'b0000: engine = 2'b10; //add
            4'b0001: engine = 2'b10; //sub
            4'b0010: engine = 2'b10; //MatMul
            4'b0011: engine = 2'b10; //addT
            4'b0100: engine = 2'b10; //subT
            4'b0101: engine = 2'b10; //loadT
            4'b0110: engine = 2'b11; //storeT
            default: engine = 2'b00; //default
 
        endcase
         //checks if load engine (01) or store engine (11) is selected
         // if load is selected, check if operand 1
         
         
         //buffer selection:
         //00: input buffer 1, 01: input buffer 2, 10: weight buffer 1, 11: weight buffer 2
         
         
         
    end
   

endmodule

