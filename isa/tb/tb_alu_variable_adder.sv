`timescale 1ns / 1ps

// Self-checking testbench for alu_variable_adder.
// Exhaustively checks all 8-bit add/sub combinations against an (N+1)-bit
// signed reference, then spot-checks the default DATA_WIDTH instance with
// directed and random vectors.
module tb_alu_variable_adder;
    localparam int N = 8;
    localparam int W = 64;  // must match `DATA_WIDTH in constants.h

    logic [N-1:0] A, B, Sum;
    logic subtract, overflow;

    alu_variable_adder #(.N(N)) dut (
        .A(A),
        .B(B),
        .subtract(subtract),
        .Sum(Sum),
        .overflow(overflow)
    );

    logic [W-1:0] A64, B64, Sum64;
    logic subtract64, overflow64;

    alu_variable_adder dut64 (
        .A(A64),
        .B(B64),
        .subtract(subtract64),
        .Sum(Sum64),
        .overflow(overflow64)
    );

    // reference: sign-extend to N+1 bits; overflow iff wide result doesn't
    // fit back into N signed bits
    task automatic check8(input logic [N-1:0] a, input logic [N-1:0] b, input logic sub);
        logic [N-1:0] exp_sum;
        logic exp_ovf;
        logic signed [N:0] wide;
        A = a;
        B = b;
        subtract = sub;
        #1;
        if (sub) begin
            exp_sum = a - b;
            wide = signed'({a[N-1], a}) - signed'({b[N-1], b});
        end
        else begin
            exp_sum = a + b;
            wide = signed'({a[N-1], a}) + signed'({b[N-1], b});
        end
        exp_ovf = (wide !== signed'({exp_sum[N-1], exp_sum}));
        if (Sum !== exp_sum)
            $fatal(1, "sum mismatch: A=%0h B=%0h sub=%0b got=%0h expected=%0h",
                   a, b, sub, Sum, exp_sum);
        if (overflow !== exp_ovf)
            $fatal(1, "overflow mismatch: A=%0h B=%0h sub=%0b got=%0b expected=%0b",
                   a, b, sub, overflow, exp_ovf);
    endtask

    task automatic check64(input logic [W-1:0] a, input logic [W-1:0] b, input logic sub);
        logic [W-1:0] exp_sum;
        logic exp_ovf;
        logic signed [W:0] wide;
        A64 = a;
        B64 = b;
        subtract64 = sub;
        #1;
        if (sub) begin
            exp_sum = a - b;
            wide = signed'({a[W-1], a}) - signed'({b[W-1], b});
        end
        else begin
            exp_sum = a + b;
            wide = signed'({a[W-1], a}) + signed'({b[W-1], b});
        end
        exp_ovf = (wide !== signed'({exp_sum[W-1], exp_sum}));
        if (Sum64 !== exp_sum)
            $fatal(1, "64-bit sum mismatch: A=%0h B=%0h sub=%0b got=%0h expected=%0h",
                   a, b, sub, Sum64, exp_sum);
        if (overflow64 !== exp_ovf)
            $fatal(1, "64-bit overflow mismatch: A=%0h B=%0h sub=%0b got=%0b expected=%0b",
                   a, b, sub, overflow64, exp_ovf);
    endtask

    localparam logic [N-1:0] MAXPOS8 = {1'b0, {(N - 1) {1'b1}}};  // +127
    localparam logic [N-1:0] MINNEG8 = {1'b1, {(N - 1) {1'b0}}};  // -128
    localparam logic [W-1:0] MAXPOS64 = {1'b0, {(W - 1) {1'b1}}};
    localparam logic [W-1:0] MINNEG64 = {1'b1, {(W - 1) {1'b0}}};

    initial begin
        // directed 8-bit cases
        check8('0, '0, 1'b0);              // 0 + 0
        check8(MAXPOS8, N'(1), 1'b0);      // max positive + 1 -> overflow
        check8(MINNEG8, N'(1), 1'b1);      // min negative - 1 -> overflow
        check8(N'(42), N'(42), 1'b1);      // A - A = 0
        check8(N'(42), '0, 1'b1);          // A - 0 = A
        check8('0, MINNEG8, 1'b1);         // 0 - (-128) -> overflow
        check8({N{1'b1}}, {N{1'b1}}, 1'b0);  // -1 + -1

        // exhaustive 8-bit sweep: all A x B x {add, sub}
        for (int a = 0; a < 2 ** N; a++) begin
            for (int b = 0; b < 2 ** N; b++) begin
                check8(N'(a), N'(b), 1'b0);
                check8(N'(a), N'(b), 1'b1);
            end
        end

        // directed + random at default DATA_WIDTH
        check64('0, '0, 1'b0);
        check64(MAXPOS64, 64'(1), 1'b0);
        check64(MINNEG64, 64'(1), 1'b1);
        check64('0, MINNEG64, 1'b1);
        for (int i = 0; i < 1000; i++) begin
            check64({$urandom(), $urandom()}, {$urandom(), $urandom()}, 1'(($urandom() & 1)));
        end

        $display("PASS: tb_alu_variable_adder");
        $finish;
    end
endmodule
