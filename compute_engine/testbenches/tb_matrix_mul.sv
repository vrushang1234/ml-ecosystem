`timescale 1ns/1ps

// Self-checking testbench for matvec_mult. Drives one vector at a time,
// waits the fixed 3-cycle latency, checks y_out, then moves to the next.
module tb_matvec_mult;
    localparam int WIDTH = 8;
    localparam int N     = 5;
    localparam int M     = 4;

    logic clk;
    logic data_valid;
    logic signed [WIDTH-1:0] x_in [N];
    logic signed [WIDTH-1:0] W_in [M][N];
    logic y_valid;
    logic signed [WIDTH-1:0] y_out [M];

    matvec_mult #(.WIDTH(WIDTH), .N(N), .M(M)) dut (
        .clk(clk), .data_valid(data_valid),
        .x_in(x_in), .W_in(W_in),
        .y_valid(y_valid), .y_out(y_out)
    );

    initial clk = 0;
    always #5 clk = ~clk;

    task automatic check(
        input logic signed [WIDTH-1:0] x [N],
        input logic signed [WIDTH-1:0] W [M][N],
        input string name
    );
        int exp_wide [M];

        @(negedge clk);
        x_in = x;
        W_in = W;
        data_valid = 1;

        @(posedge clk);           // v1 <= 1, x_reg <= x_in
        data_valid <= 0;          // nonblocking: guarantees the DUT's
                                   // same-edge read of data_valid still
                                   // sees the pre-edge value (1), regardless
                                   // of which Active-region process runs first
        @(posedge clk);           // v2 <= 1, prod_reg <= W*x_reg
        @(posedge clk);           // y_valid <= 1, y_out <= acc
        #1;                       // let the DUT's nonblocking assigns settle
                                   // before we read y_valid/y_out (avoids a
                                   // read-before-NBA-update race)

        if (!y_valid)
            $fatal(1, "%s: y_valid not asserted after 3-cycle latency", name);
        //LOOP for manual checking of the output
        for (int i = 0; i < M; i++) begin
            exp_wide[i] = 0;
            for (int k = 0; k < N; k++) exp_wide[i] += W[i][k] * x[k];
            if (y_out[i] !== WIDTH'(exp_wide[i]))
                $fatal(1, "%s: y_out[%0d] mismatch: got=%0d expected=%0d",
                       name, i, y_out[i], WIDTH'(exp_wide[i]));
        end
        $display("PASS: %s", name);
    endtask

    initial begin
        data_valid = 0;

        // zero case
        begin
            logic signed [WIDTH-1:0] x0 [N] = '{default: '0};
            logic signed [WIDTH-1:0] W0 [M][N] = '{default: '0};
            check(x0, W0, "all_zero");
        end

        // positive, one-hot rows -> y[i] = x[i]
        begin
            logic signed [WIDTH-1:0] x1 [N] = '{1, 2, 3, 4, 5};
            logic signed [WIDTH-1:0] W1 [M][N] = '{
                '{1, 0, 0, 0, 0},
                '{0, 1, 0, 0, 0},
                '{0, 0, 1, 0, 0},
                '{0, 0, 0, 1, 0}
            };
            check(x1, W1, "one_hot_rows");
        end

        // negative x, all-ones rows -> exercises sign extension
        begin
            logic signed [WIDTH-1:0] x2 [N] = '{-1, 2, -3, 4, -5};
            logic signed [WIDTH-1:0] W2 [M][N] = '{
                '{1, 1, 1, 1, 1},
                '{1, 1, 1, 1, 1},
                '{1, 1, 1, 1, 1},
                '{1, 1, 1, 1, 1}
            };
            check(x2, W2, "negative_x_sign_check");
        end

        // small random sweep
        for (int t = 0; t < 10; t++) begin
            logic signed [WIDTH-1:0] xr [N];
            logic signed [WIDTH-1:0] Wr [M][N];
            for (int i = 0; i < N; i++) xr[i] = WIDTH'($urandom());
            for (int i = 0; i < M; i++)
                for (int k = 0; k < N; k++) Wr[i][k] = WIDTH'($urandom());
            check(xr, Wr, $sformatf("random_%0d", t));
        end

        $display("PASS: tb_matvec_mult");
        $finish;
    end
endmodule