`timescale 1ns / 1ps

// Self-checking testbench for decode.
// Covers: opcode/engine routing for every non-store opcode crossed with all
// op1/op2 combinations; the full store-field extraction (scratch/offset/
// row/col/stride/acc) for every 14-bit field pattern under opcode storeT;
// and the always_latch hold-over of op1/op2 across a store instruction
// (decode.sv only drives op1/op2 on the non-store path, so they must retain
// their last value rather than reset).
module tb_decode;
    logic [31:0] instruction;
    logic [3:0]  opcode;
    logic [1:0]  engine, op1, op2;
    logic [1:0]  scratch, offset, row, col, acc;
    logic [3:0]  stride;

    decode dut (
        .instruction(instruction),
        .opcode(opcode),
        .engine(engine),
        .op1(op1),
        .op2(op2),
        .scratch(scratch),
        .offset(offset),
        .row(row),
        .col(col),
        .stride(stride),
        .acc(acc)
    );

    function automatic logic [1:0] engine_for(input logic [3:0] op);
        case (op)
            // ALU engine: add, sub, MatMul, addT, subT, loadT
            4'b0000, 4'b0001, 4'b0010, 4'b0011, 4'b0100, 4'b0101: return 2'b10;
            4'b0110: return 2'b11;  // storeT
            default: return 2'b00;
        endcase
    endfunction

    task automatic drive(input logic [31:0] instr);
        instruction = instr;
        #1;
    endtask

    task automatic check_common(input logic [31:0] instr);
        logic [3:0] exp_opcode;
        exp_opcode = instr[31:28];
        if (opcode !== exp_opcode)
            $fatal(1, "opcode mismatch: instr=%08h got=%04b expected=%04b",
                   instr, opcode, exp_opcode);
        if (engine !== engine_for(exp_opcode))
            $fatal(1, "engine mismatch: instr=%08h opcode=%04b got=%02b expected=%02b",
                   instr, exp_opcode, engine, engine_for(exp_opcode));
    endtask

    task automatic check_nonstore(input logic [31:0] instr);
        drive(instr);
        check_common(instr);
        if (op1 !== instr[3:2])
            $fatal(1, "op1 mismatch: instr=%08h got=%02b expected=%02b", instr, op1, instr[3:2]);
        if (op2 !== instr[1:0])
            $fatal(1, "op2 mismatch: instr=%08h got=%02b expected=%02b", instr, op2, instr[1:0]);
        if (scratch !== 2'b00 || offset !== 2'b00 || row !== 2'b00 || col !== 2'b00 ||
            stride !== 4'b0000 || acc !== 2'b00)
            $fatal(1, "store fields not cleared on non-store opcode: instr=%08h", instr);
    endtask

    task automatic check_store(input logic [31:0] instr);
        drive(instr);
        check_common(instr);
        if (scratch !== instr[13:12])
            $fatal(1, "scratch mismatch: instr=%08h got=%02b expected=%02b",
                   instr, scratch, instr[13:12]);
        if (offset !== instr[11:10])
            $fatal(1, "offset mismatch: instr=%08h got=%02b expected=%02b",
                   instr, offset, instr[11:10]);
        if (row !== instr[9:8])
            $fatal(1, "row mismatch: instr=%08h got=%02b expected=%02b", instr, row, instr[9:8]);
        if (col !== instr[7:6])
            $fatal(1, "col mismatch: instr=%08h got=%02b expected=%02b", instr, col, instr[7:6]);
        if (stride !== instr[5:2])
            $fatal(1, "stride mismatch: instr=%08h got=%04b expected=%04b",
                   instr, stride, instr[5:2]);
        if (acc !== instr[1:0])
            $fatal(1, "acc mismatch: instr=%08h got=%02b expected=%02b", instr, acc, instr[1:0]);
    endtask

    initial begin
        // 1. Every non-store opcode (0-15 minus storeT=0110) crossed with all
        //    op1/op2 combinations. Don't-care bits randomized to catch any
        //    accidental use of bits outside opcode/op1/op2.
        for (int op = 0; op < 16; op++) begin
            if (op[3:0] == 4'b0110) continue;
            for (int a = 0; a < 4; a++) begin
                for (int b = 0; b < 4; b++) begin
                    check_nonstore({op[3:0], 24'($urandom()), a[1:0], b[1:0]});
                end
            end
        end

        // 2. Exhaustive store-field sweep: all 14 bits feeding
        //    scratch/offset/row/col/stride/acc, opcode fixed to storeT.
        //    Don't-care middle bits randomized.
        for (int fields = 0; fields < 16384; fields++) begin
            check_store({4'b0110, 14'($urandom()), fields[13:0]});
        end

        // 3. Latch-hold check: decode.sv only drives op1/op2 on the
        //    non-store path (always_latch), so a storeT instruction must
        //    leave op1/op2 holding their last non-store value.
        begin
            logic [1:0]  held_op1, held_op2;
            logic [31:0] store_instr;
            check_nonstore({4'b0000, 24'($urandom()), 2'b10, 2'b01});
            held_op1 = op1;
            held_op2 = op2;
            store_instr = {4'b0110, 28'($urandom())};
            drive(store_instr);
            check_common(store_instr);
            if (op1 !== held_op1 || op2 !== held_op2)
                $fatal(1, "op1/op2 changed on store opcode (expected latch hold): got op1=%02b op2=%02b expected op1=%02b op2=%02b",
                       op1, op2, held_op1, held_op2);
        end

        $display("PASS: tb_decode");
        $finish;
    end
endmodule
