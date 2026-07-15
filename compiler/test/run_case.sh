#!/usr/bin/env bash
# Integration test runner for the compiler CLI.
# Usage: run_case.sh <compiler-bin> <fixture.nn> <expected-exit> [expected-stderr-regex]
set -u

bin=$1
fixture=$2
want_exit=$3
regex=${4:-}

stderr_out=$("$bin" "$fixture" 2>&1 >/dev/null)
got_exit=$?

if [ "$got_exit" -ne "$want_exit" ]; then
    echo "FAIL: '$fixture' exited $got_exit, expected $want_exit"
    echo "--- stderr ---"
    echo "$stderr_out"
    exit 1
fi

if [ -n "$regex" ] && ! printf '%s' "$stderr_out" | grep -Eq "$regex"; then
    echo "FAIL: '$fixture' stderr missing pattern: $regex"
    echo "--- stderr ---"
    echo "$stderr_out"
    exit 1
fi

echo "PASS: $fixture (exit $got_exit)"
