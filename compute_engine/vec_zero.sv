`timescale 1ns / 1ps



module vec_zero #(
    parameter int WIDTH = 8,
    parameter int M     = 4
) (
    input  logic clear,   //if clear=0-> DOESN't change the value 
                          //if clear=1-> Changes the value to all 0s
    input  logic signed [WIDTH-1:0] in  [M],
    output logic signed [WIDTH-1:0] out [M]
);
    always_comb begin
        for (int i = 0; i < M; i++) begin
            out[i] = clear ? '0 : in[i];
        end
    end
endmodule