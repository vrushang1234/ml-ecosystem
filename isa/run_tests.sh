#!/usr/bin/env bash
# Lints all RTL modules with Verilator, then builds and runs each
# self-checking testbench (verilator --binary). Fails on the first error.
set -euo pipefail
cd "$(dirname "$0")"

INC="-Iadder"

echo "=== Lint ==="
verilator --lint-only -Wall $INC verilator.vlt adder/alu_variable_adder.sv
verilator --lint-only -Wall $INC verilator.vlt adder/negation_module.sv
verilator --lint-only -Wall $INC verilator.vlt decode/decode.sv

run_tb() {
    local top=$1
    shift
    echo "=== $top ==="
    mkdir -p "obj_dir/$top"
    verilator --binary --timing -Wall -Wno-fatal $INC verilator.vlt \
        --top-module "$top" --Mdir "obj_dir/$top" -o "$top" "$@"
    "obj_dir/$top/$top"
}

run_tb tb_alu_variable_adder tb/tb_alu_variable_adder.sv adder/alu_variable_adder.sv
run_tb tb_negation_module tb/tb_negation_module.sv adder/negation_module.sv
run_tb tb_decode tb/tb_decode.sv decode/decode.sv

echo "ALL ISA TESTS PASSED"
