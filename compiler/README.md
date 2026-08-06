# Compiler

Compiles ONNX models into a tiled, buffer-explicit instruction stream for the ISA in `../isa`.

## Pipeline

```
model.onnx → parser → AST + constant pool → sema → ResolvedNetwork → TIR → MIR
```

| Stage    | Flag              | What it is                                                              |
| -------- | ----------------- | ----------------------------------------------------------------------- |
| AST      | `--emit=ast`      | ONNX graph lowered to networks/layers, plus the quantized constant pool |
| Resolved | `--emit=resolved` | Layer sizes reconciled and activations validated                        |
| TIR      | `--emit=tir`      | SSA over whole tensors; layers expanded into primitive ops              |
| MIR      | `--emit=mir`      | Tiled, buffer-explicit, fully unrolled instruction list (default)       |

```
bin/compiler --emit=mir test/test.onnx
bin/compiler --format=q24.8 --emit=ast test/quant.onnx
```

The ONNX subset supported is a chain of `Gemm` nodes, each optionally followed by an activation
node. The protobuf wire format is decoded directly — there is no libprotobuf or onnx dependency.

## Numerics

Weights are quantized at parse time to a fixed-point format, `q<int>.<frac>`, default `q16.16` in
32-bit storage. A value that does not fit the chosen format is a hard error rather than a saturating
convert, because a silently clamped weight produces a model that runs and gives wrong answers.

The accumulator is twice as wide in both parts (`q32.32` in 64 bits) and holds raw products. That
choice fixes two shifts the datapath owes, both printed in the MIR header:

- `add` shifts its Input-buffer operand **left** by `fracBits` (bias is `q16.16`, accumulator is `q32.32`)
- `storeT` shifts the accumulator **right** by `fracBits` on the way back to storage width

## MIR reference

```
zero   ACC                                  ACC = 0
loadT  <buf>, <memref>                      buffer = tile, zero-filled past the clipped extent
storeT <memref>, ACC                        memory = ACC >> fracBits
matmul ACC, <wbuf>, <ibuf>                  ACC += wbuf · ibuf   (raw products, no shift)
add    ACC, ACC, <ibuf>                     ACC += ibuf << fracBits
relu   ACC                                  ACC = max(ACC, 0)
softmax <memref>                            in-place reduction over a whole vector
```

A memref is `@symbol[offset RxC stride S]` for the constant pool and `%symbol[...]` for scratch.
`stride` is the row pitch of the _full_ tensor, so a tile of a row-major matrix is addressable
without relaying out the pool. Tiles at the edge of a tensor are **clipped**, not padded — `loadT`
is responsible for zero-filling the rest of the buffer, which keeps the instruction count uniform.

Every engine buffer is double-buffered for ping-pong. Input and weight buffers alternate on the K
loop (`IB0`/`IB1`, `WB0`/`WB1`) so a load can overlap the matmul consuming the other buffer. The two
accumulators (`ACC0`/`ACC1`) alternate on the M loop, so the store of one M-tile's result can run
against the compute of the next. The IR makes all of this explicit rather than leaving it to a later
scheduler.

Each buffer holds one tile today (`.buffers ... elems=` in the header). `MachineModel::bufferElems`
is the hook for sizing them from the FPGA's SRAM spec later; when a whole activation vector fits on
chip, an accumulator could hand off straight to an input buffer and skip the DRAM round-trip. That
residency optimization is not implemented yet — see the ISA list below.

## What the ISA still needs

The MIR assumes hardware that does not exist yet in `../isa`. In rough order of blocking severity:

1. **Instruction width.** `loadT`/`storeT` need an address. A 12-bit instruction with `[7:4]` opcode
   and `[3:0]` operands leaves only `[11:8]`, which addresses 16 words; `test.onnx` alone needs ~93
   constant words. Either widen the instruction with an immediate field, or make load/store
   two-word. The IR printer is deliberately encoding-agnostic so this can be settled in RTL.
2. **Multiply.** Opcode `0010` (MatMul) is decoded in `decode.sv` but no multiply engine exists —
   only `alu_variable_adder.sv` and `negation_module.sv`. A `tileM × tileK` MAC array is required.
3. **Accumulator.** `matmul`/`add`/`relu` target an `ACC` that is wider than the buffers and
   persists across the K loop, plus a `zero ACC` op. There are two (`ACC0`/`ACC1`) so a store can
   overlap the next tile's compute; the operand field must select which. `decode.sv` has no
   accumulator concept and no third operand field.
4. **Fixed-point shifts.** The `add<<` and `storeT>>` behaviour described above. Hardwire it, or
   carry the shift amount as an instruction field if the format stays runtime-configurable.
5. **Strided loads.** `loadT` of a weight tile reads `rows` runs of `cols` elements separated by
   `stride`. If a strided load is expensive in hardware, the alternative is a compiler pass that
   rewrites the constant pool into tile-major order so every `loadT` becomes a flat burst. That pass
   does not exist yet; the trade-off is real and worth deciding before the RTL is written.
6. **Activation opcodes.** `relu` is cheap (compare against zero, select). `softmax` needs `exp` and
   a reciprocal — realistically a LUT plus a divide, and it is by far the most expensive item here.
   Worth deciding whether softmax stays on-device at all, or the compiler emits raw logits and
   softmax happens off-device.
7. **Buffer count.** Two input, two weight, and two accumulator buffers is what the tiling assumes.
   Changing the count means changing `MachineModel` and widening the 2-bit operand fields.
8. **Buffer capacity (future).** Buffers hold one tile today. Once sized from the FPGA's SRAM spec
   (`MachineModel::bufferElems`), a residency pass can keep small activations on chip and let an
   accumulator drain directly into an input buffer, dropping the DRAM store/load between layers.

## Tests

```
cmake -S . -B cmake-build && cmake --build cmake-build
ctest --test-dir cmake-build --output-on-failure
```

Error-path cases check the exit code and a stderr pattern; golden cases diff stdout against
`test/expected/`. Regenerate goldens after an intentional IR change with
`REGEN=1 test/run_golden.sh bin/compiler test/test.onnx mir test/expected/golden_mir_basic.txt`.

Fixtures are generated by `test/make_fixtures.py`, which hand-packs the ONNX protobuf wire format
using only the standard library. Regenerate with `python3 test/make_fixtures.py`.
