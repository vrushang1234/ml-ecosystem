`timescale 1ns / 1ps

// Self-checking testbench for relu_unit. Combinational DUT, no clock needed.
module tb_relu_unit;
    localparam int WIDTH = 8;
    localparam int M     = 4;

    logic signed [WIDTH-1:0] in  [M];
    logic signed [WIDTH-1:0] out [M];

    relu_unit #(.WIDTH(WIDTH), .M(M)) dut (
        .in(in),
        .out(out)
    );

    task automatic check(
        input logic signed [WIDTH-1:0] in_vals [M],
        input string name
    );
        in = in_vals;
        #1;

        for (int i = 0; i < M; i++) begin
            logic signed [WIDTH-1:0] expected = (in_vals[i][WIDTH-1]) ? '0 : in_vals[i];
            if (out[i] !== expected)
                $fatal(1, "%s: out[%0d] mismatch: got=%0d expected=%0d",
                       name, i, out[i], expected);
        end
        $display("PASS: %s", name);
    endtask

    initial begin
        check('{1, 2, 3, 4},         "all_positive");
        check('{-1, -2, -3, -4},     "all_negative");
        check('{0, 0, 0, 0},         "all_zero");
        check('{-1, 2, -3, 4},       "mixed_sign");
        check('{127, -128, 0, -1},   "boundary_values"); // max for 8 bits

        for (int t = 0; t < 10; t++) begin
            logic signed [WIDTH-1:0] rand_in [M];
            for (int i = 0; i < M; i++) rand_in[i] = WIDTH'($urandom());
            check(rand_in, $sformatf("random_%0d", t));
        end

        $display("PASS: tb_relu_unit");
        $finish;
    end
endmodule