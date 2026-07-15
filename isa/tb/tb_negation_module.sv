`timescale 1ns / 1ps

// Self-checking testbench for negation_module.
// Verifies operand_B == two's complement of B for edge cases and random vectors.
module tb_negation_module;
    localparam int N = 8;

    logic [N-1:0] B;
    logic [N-1:0] operand_B;

    negation_module #(.N(N)) dut (
        .B(B),
        .operand_B(operand_B)
    );

    task automatic check(input logic [N-1:0] value);
        logic [N-1:0] expected;
        B = value;
        #1;
        expected = ~value + 1'b1;
        if (operand_B !== expected)
            $fatal(1, "negation mismatch: B=%0h got=%0h expected=%0h", value, operand_B, expected);
    endtask

    initial begin
        // edge cases: zero, one, all ones, most-negative (MSB only)
        check('0);
        check(N'(1));
        check({N{1'b1}});
        check({1'b1, {(N - 1) {1'b0}}});
        // exhaustive for N=8
        for (int i = 0; i < 2 ** N; i++) begin
            check(N'(i));
        end
        $display("PASS: tb_negation_module");
        $finish;
    end
endmodule
