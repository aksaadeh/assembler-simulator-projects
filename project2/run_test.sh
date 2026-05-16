#!/bin/bash

echo "Running tests..."
echo "------------------"

make
if [ $? -ne 0 ]; then
    echo "Build failed. Aborting."
    exit 1
fi

for lcfile in programs/*.lc; do
    base=$(basename "$lcfile" .lc)

    ./assembler "$lcfile" > "programs/$base.hex"
    xxd -r -p "programs/$base.hex" > "programs/$base.bin"
done

for input in tests/*.in; do
    base=$(basename "$input" .in)
    expected="tests/$base.out"
    actual="tests/$base.actual"

    ./simulator 6 < "$input" > "$actual"

    if diff -q "$actual" "$expected" > /dev/null; then
        echo "$base: PASS"
    else
        echo "$base: FAIL"
        echo "Differences:"
        diff "$actual" "$expected"
    fi
done

echo "------------------"
echo "Done"