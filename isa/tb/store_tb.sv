`timescale 1ns / 1ps

module store_tb;

    // ---- DUT inputs: we DRIVE these → logic ----
    logic clk, m_axi_aresetn, start;
    logic [15:0] addr_in;
    logic [31:0] data_in;
    logic [1:0]  src_buffer;
    logic m_axi_awready, m_axi_wready, m_axi_bvalid;
    logic [1:0] m_axi_bresp;
    
    // read inputs you'd "drive" (they're DUT inputs) → logic
    logic        m_axi_arready, m_axi_rvalid;
    logic [31:0] m_axi_rdata;
    logic [1:0]  m_axi_rresp;
    // read outputs from DUT → wire
    wire         m_axi_arvalid, m_axi_rready;
    wire [31:0]  m_axi_araddr;
    
    

    // ---- DUT outputs: we OBSERVE these → wire ----
    wire m_axi_awvalid, m_axi_wvalid, m_axi_bready;
    wire [31:0] m_axi_awaddr, m_axi_wdata;
    wire [3:0] m_axi_wstrb;

    // ---- instantiate the DUT: every port wired ----
    store dut (
        .clk(clk), .m_axi_aresetn(m_axi_aresetn), .start(start),
        .addr_in(addr_in), .src_buffer(src_buffer), .data_in(data_in),
        .m_axi_awready(m_axi_awready), .m_axi_wready(m_axi_wready),
        .m_axi_bvalid(m_axi_bvalid),   .m_axi_bresp(m_axi_bresp),
        .m_axi_awvalid(m_axi_awvalid), .m_axi_awaddr(m_axi_awaddr),
        .m_axi_wvalid(m_axi_wvalid),   .m_axi_wdata(m_axi_wdata),
        .m_axi_wstrb(m_axi_wstrb),     .m_axi_bready(m_axi_bready),
        .m_axi_arready(m_axi_arready),  .m_axi_rvalid(m_axi_rvalid),
        .m_axi_rdata(m_axi_rdata), .m_axi_rresp(m_axi_rresp)
    );

    // ---- clock: 10 ns period ----
    initial clk = 1'b0;
    always #5 clk = ~clk;

    // ---- happy-path fake slave: always ready, always OKAY ----
    initial begin
        m_axi_awready = 1'b1;
        m_axi_wready  = 1'b1;
        m_axi_bvalid  = 1'b1;
        m_axi_bresp   = 2'b00;   // OKAY
        m_axi_arready = 1'b0;
    m_axi_rvalid  = 1'b0;
    m_axi_rdata   = 32'b0;
    m_axi_rresp   = 2'b00;
    end

    // ---- live log of what's happening ----
    initial begin
        $monitor("t=%0t  state=%0d awvalid=%b awaddr=%h  wvalid=%b wdata=%h  bready=%b",
         $time, dut.state, m_axi_awvalid, m_axi_awaddr,
         m_axi_wvalid, m_axi_wdata, m_axi_bready);
    end

    // ---- stimulus ----
    // ---- stimulus ----
initial begin
    // 1) reset (Blocking is okay here for initialization at time 0)
    m_axi_aresetn = 1'b0;  
    start = 1'b0;
    addr_in = 16'h0; 
    data_in = 32'h0; 
    src_buffer = 2'b0;
    
    repeat (2) @(posedge clk);
    m_axi_aresetn <= 1'b1; // Use <= moving forward in synchronization with the clock
    @(posedge clk);
    
    // 2) present inputs, pulse start for exactly one cycle
    addr_in    <= 16'hABCD;
    data_in    <= 32'hDEADBEEF;
    src_buffer <= 2'b01;
    start      <= 1'b1;     
    @(posedge clk);
    start      <= 1'b0;     

    // 3) check the payload as the handshakes happen
    wait (m_axi_awvalid);
    if (m_axi_awaddr !== 32'h0000ABCD)
        $error("awaddr wrong: got %h expected 0000ABCD", m_axi_awaddr);
    wait (m_axi_wvalid);
    if (m_axi_wdata !== 32'hDEADBEEF)
        $error("wdata wrong: got %h expected DEADBEEF", m_axi_wdata);

    // 4) let it return to IDLE, then finish
    repeat (6) @(posedge clk);
    $display("PASS: happy-path write completed");
    $finish;
end

endmodule