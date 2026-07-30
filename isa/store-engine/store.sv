`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 07/25/2026 03:24:08 PM
// Design Name: 
// Module Name: store
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision: 
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module store(
    input logic clk,
    input logic rst, 
    input logic start,
    input logic [15:0] addr_in,  // needs to come from decode.sv output
    input logic [1:0] src_buffer, //also from decode.sv output
    input logic [31:0] data_in,  //assuming 32 bit data in buffer 
    
    //Write address channel
    output logic [31:0] m_axi_awaddr,
    output logic  m_axi_awvalid,
    input logic m_axi_awready,
    
    //Write data channel
    output logic [31:0] m_axi_wdata,
    output logic [3:0]  m_axi_wstrb,
    output logic m_axi_wvalid,
    input logic m_axi_wready,
    
    //Write response channel
    input logic [1:0] m_axi_bresp,
    input logic m_axi_bvalid,
    output logic m_axi_bready
    );
    
    
    typedef enum logic [1:0] {IDLE, SEND_ADDR, SEND_DATA, WAIT_RESP} state_t;
    state_t state;
    state_t next_state;
    logic [15:0] addr_q;
    logic [31:0] data_q;
    logic [1:0] src_buffer_q;
    
    always_ff @(posedge clk) begin
        if (rst) begin
            state <= IDLE;
        end else begin
            state <= next_state;
            if (state == IDLE) begin
                if (start) begin
                    addr_q <= addr_in;
                    data_q <= data_in;
                end
            end
        end
    end
    
    always_comb begin
        next_state = state;
        m_axi_awvalid = 1'b0;             
        m_axi_wvalid  = 1'b0;
        m_axi_bready  = 1'b0;
        m_axi_awaddr  = {16'b0, addr_q};  
        m_axi_wdata   = data_q;
        m_axi_wstrb   = 4'b1111; 
        
        case (state)
            IDLE: begin
                if (start) begin
                    next_state = SEND_ADDR;
                end
            end
            SEND_ADDR: begin
                m_axi_awvalid = 1'b1;
                if (m_axi_awready)
                    next_state = SEND_DATA;
            end
            
            SEND_DATA: begin
                m_axi_wvalid = 1'b1;
                if (m_axi_wready)
                    next_state = WAIT_RESP;
            end
            WAIT_RESP: begin
                m_axi_bready = 1'b1;
                if (m_axi_bvalid)
                    next_state = IDLE;
            end
            default: next_state = IDLE;    
        endcase
            
    
    end
                

endmodule
