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
    //instruction defualt is 0000
    input logic [3:0] instruction,
    output logic subtract
);
    always_comb begin
        subtract = 1'b0;
        
        case (instruction)
            4'b0000: subtract = 1'b0;
            4'b0001: subtract = 1'b1;
            default: subtract = 1'b0;
        endcase
    end
 
    
endmodule
