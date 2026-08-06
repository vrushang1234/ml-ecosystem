#!/usr/bin/env bash
# Golden-output test runner for the compiler CLI.
# Usage: run_golden.sh <compiler-bin> <fixture.onnx> <stage> <expected-file>
# Set REGEN=1 to rewrite the expected file instead of comparing.
set -u

bin=$1
fixture=$2
stage=$3
expected=$4

actual=$("$bin" "--emit=$stage" "$fixture" 2>/dev/null)
got_exit=$?

if [ "$got_exit" -ne 0 ]; then
    echo "FAIL: '$fixture' (--emit=$stage) exited $got_exit"
    "$bin" "--emit=$stage" "$fixture" >/dev/null
    exit 1
fi

if [ "${REGEN:-0}" = "1" ]; then
    printf '%s\n' "$actual" > "$expected"
    echo "REGEN: $expected"
    exit 0
fi

if ! printf '%s\n' "$actual" | diff -u "$expected" - > /tmp/golden_diff.$$ 2>&1; then
    echo "FAIL: '$fixture' (--emit=$stage) does not match $expected"
    cat /tmp/golden_diff.$$
    rm -f /tmp/golden_diff.$$
    exit 1
fi

rm -f /tmp/golden_diff.$$
echo "PASS: $fixture (--emit=$stage)"
