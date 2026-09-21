`timescale 1ns / 1ps

// Self-checking testbench for vec_add. Combinational DUT, no clock needed.
module tb_vec_add;
    localparam int WIDTH = 8;
    localparam int M     = 4;

    logic signed [WIDTH-1:0] A [M];
    logic signed [WIDTH-1:0] B [M];
    logic signed [WIDTH-1:0] Sum [M];
    logic overflow [M];

    vec_add #(.WIDTH(WIDTH), .M(M)) dut (
        .A(A), .B(B), .Sum(Sum), .overflow(overflow)
    );

    task automatic check(
        input logic signed [WIDTH-1:0] a [M],
        input logic signed [WIDTH-1:0] b [M],
        input string name
    );
        A = a;
        B = b;
        #1;

        for (int i = 0; i < M; i++) begin
            logic signed [WIDTH-1:0] expected = WIDTH'(a[i] + b[i]);
            if (Sum[i] !== expected)
                $fatal(1, "%s: Sum[%0d] mismatch: got=%0d expected=%0d",
                       name, i, Sum[i], expected);
        end
        $display("PASS: %s", name);
    endtask

    initial begin
        check('{1, 2, 3, 4}, '{10, 20, 30, 40}, "basic_positive");
        check('{-1, -2, -3, -4}, '{1, 2, 3, 4}, "negatives_cancel");
        check('{127, 127, 127, 127}, '{1, 1, 1, 1}, "overflow_boundary");
        check('{-128, -128, -128, -128}, '{-1, -1, -1, -1}, "underflow_boundary");

        for (int t = 0; t < 10; t++) begin
            logic signed [WIDTH-1:0] ra [M];
            logic signed [WIDTH-1:0] rb [M];
            for (int i = 0; i < M; i++) begin
                ra[i] = WIDTH'($urandom());
                rb[i] = WIDTH'($urandom());
            end
            check(ra, rb, $sformatf("random_%0d", t));
        end

        $display("PASS: tb_vec_add");
        $finish;
    end
endmodule