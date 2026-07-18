`timescale 1ns / 1ps

// Self-checking testbench for decode.
// Exhaustively drives all 4096 instruction encodings and checks the field
// split plus the engine routing table, including the store-engine override
// (storeT with op1[1] set routes to engine 00).
module tb_decode;
    logic [11:0] instruction;
    logic [3:0] opcode;
    logic [1:0] engine, op1, op2;
    /* verilator lint_off UNUSEDSIGNAL */
    logic out;  // declared by decode but never driven (known RTL issue)
    /* verilator lint_on UNUSEDSIGNAL */

    decode dut (
        .instruction(instruction),
        .opcode(opcode),
        .engine(engine),
        .op1(op1),
        .op2(op2),
        .out(out)
    );

    logic [1:0] exp_engine;

    initial begin
        for (int i = 0; i < 4096; i++) begin
            instruction = 12'(i);
            #1;
            if (opcode !== instruction[7:4])
                $fatal(1, "opcode mismatch: instr=%012b got=%04b expected=%04b",
                       instruction, opcode, instruction[7:4]);
            if (op1 !== instruction[3:2])
                $fatal(1, "op1 mismatch: instr=%012b got=%02b expected=%02b",
                       instruction, op1, instruction[3:2]);
            if (op2 !== instruction[1:0])
                $fatal(1, "op2 mismatch: instr=%012b got=%02b expected=%02b",
                       instruction, op2, instruction[1:0]);

            case (instruction[7:4])
                // ALU engine: add, sub, MatMul, addT, subT, loadT
                4'b0000, 4'b0001, 4'b0010, 4'b0011, 4'b0100, 4'b0101: exp_engine = 2'b10;
                // storeT, unless op1 selects a weight buffer (op1[1] set)
                4'b0110: exp_engine = instruction[3] ? 2'b00 : 2'b11;
                default: exp_engine = 2'b00;
            endcase
            if (engine !== exp_engine)
                $fatal(1, "engine mismatch: instr=%012b got=%02b expected=%02b",
                       instruction, engine, exp_engine);
        end
        $display("PASS: tb_decode");
        $finish;
    end
endmodule
