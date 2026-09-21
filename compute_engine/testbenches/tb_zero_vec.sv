`timescale 1ns / 1ps

// Self-checking testbench for vec_zero.
// Pure combinational DUT: drive inputs, let them settle, check.
module tb_vec_zero;
    localparam int WIDTH = 8;
    localparam int M     = 4;

    logic clear;
    logic signed [WIDTH-1:0] in  [M];
    logic signed [WIDTH-1:0] out [M];

    vec_zero #(.WIDTH(WIDTH), .M(M)) dut (
        .clear(clear),
        .in(in),
        .out(out)
    );

    task automatic check(
        input logic clear_val,
        input logic signed [WIDTH-1:0] in_vals [M],
        input string name
    );
        clear = clear_val;
        in    = in_vals;
        #1;   // let combinational logic settle

        for (int i = 0; i < M; i++) begin
            logic signed [WIDTH-1:0] expected = clear_val ? '0 : in_vals[i];
            if (out[i] !== expected)
                $fatal(1, "%s: out[%0d] mismatch: got=%0d expected=%0d",
                       name, i, out[i], expected);
        end
        $display("PASS: %s", name);
    endtask

    initial begin
        // pass-through cases (clear = 0)
        check(1'b0, '{1, 2, 3, 4},       "passthrough_positive");
        check(1'b0, '{-1, -2, -3, -4},   "passthrough_negative");
        check(1'b0, '{0, 0, 0, 0},       "passthrough_zero");

        // clear cases (clear = 1) -- output should be 0 regardless of `in`
        check(1'b1, '{1, 2, 3, 4},       "clear_forces_zero_positive");
        check(1'b1, '{-5, 100, -128, 127}, "clear_forces_zero_negative");
        check(1'b1, '{0, 0, 0, 0},       "clear_forces_zero_already_zero");

        // small random sweep, both clear states
        for (int t = 0; t < 10; t++) begin
            logic signed [WIDTH-1:0] rand_in [M];
            for (int i = 0; i < M; i++) rand_in[i] = WIDTH'($urandom());
            check($urandom_range(0, 1), rand_in, $sformatf("random_%0d", t));
        end

        $display("PASS: tb_vec_zero");
        $finish;
    end
endmodule