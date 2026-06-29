`timescale 1ns / 1ps

`include "constants.h"

module tb_alu_variable_adder();

reg [`DATA_WIDTH-1:0] tb_A;
reg [`DATA_WIDTH-1:0] tb_B;
reg  tb_operation;
wire [`DATA_WIDTH-1:0] tb_Result;
wire  tb_over_flow;

alu_variable_adder uut (

       .A(tb_A), 
       .B(tb_B),
       .subtract(tb_operation),
       .Sum(tb_Result),
       .overflow(tb_over_flow)
    ); 

initial begin 
    tb_A=0 ;
    tb_B=0;
     tb_operation= 1;
    #100 
    
 // max positive + 1
    tb_A = 64'h7FFFFFFFFFFFFFFF; 
    tb_B = 64'd1; 
    tb_operation= 0;
    #100;
    
    // 0 -1
    tb_A = 64'h8000000000000000; 
    tb_B = 64'd1; 
    tb_operation = 1; 
    #100;
    
    $finish; 
    end 


endmodule
