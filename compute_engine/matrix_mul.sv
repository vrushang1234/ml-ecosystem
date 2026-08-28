module matvec_mult #(
    parameter int WIDTH = 8,
    parameter int N     = 5,
    parameter int M     = 4
) (
    input  logic clk,
    input  logic data_valid,  
    //coming from the compute engine, used to make sure that the y_output doesn't use the garbage 
    input  logic signed [WIDTH-1:0] x_in     [N],
    input  logic signed [WIDTH-1:0] W_in     [M][N],
    output logic y_valid,
    output logic signed [WIDTH-1:0] y_out    [M]
);
    logic v1, v2;
    logic signed [WIDTH-1:0] x_reg [N];
    logic signed [WIDTH-1:0] prod_reg [M][N];
    
    
    always_ff @(posedge clk) begin
        v1 <= data_valid;
        if (data_valid) begin
            x_reg <= x_in;
        end
    end
    
    
    always_ff @(posedge clk) begin
        v2 <= v1;
        if (v1) begin
            for (int i = 0; i < M; i++) begin
                for (int k = 0; k < N; k++) begin
                    prod_reg[i][k] <= W_in[i][k] * x_reg[k];
                end
            end
        end
    end
    
    
    always_ff @(posedge clk) begin
        y_valid <= v2;
        if (v2) begin
            for (int i = 0; i < M; i++) begin
                logic signed [WIDTH-1:0] acc; 
                acc = '0;
                for (int k = 0; k < N; k++) begin
                    acc = acc + prod_reg[i][k];
                end
                y_out[i] <= acc;
            end
        end
    end
endmodule