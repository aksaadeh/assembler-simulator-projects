#!/bin/bash

ASSEMBLER=./assembler
SIM=./simulator

if [ ! -f "$ASSEMBLER" ]; then
    echo "Assembler not found!"
    exit 1
fi

if [ ! -f "$SIM" ]; then
    echo "Simulator not found!"
    exit 1
fi

echo "=== Building binaries ==="

for file in test/*.lc; do
    name=$(basename "$file" .lc)

    echo "Building $name..."

    $ASSEMBLER "$file" > "test/$name.hex" || exit 1
    xxd -r -p "test/$name.hex" > "test/$name.bin" || exit 1
done

echo "=== Running tests ==="

for input in test/*.in; do
    name=$(basename "$input" .in)

    echo "------------------------"
    echo "Test: $name"

    $SIM 12 1 1 < "$input" > "test/$name.out"

    if [ -f "test/$name.expected" ]; then
        diff_output=$(diff --suppress-common-lines -y \
            "test/$name.expected" "test/$name.out")

        if [ -z "$diff_output" ]; then
            echo " PASS"
        else
            echo "FAIL — Differences:"
            echo "$diff_output"
        fi
    else
        echo "No expected file for $name"
    fi
done

echo "=== Done ==="