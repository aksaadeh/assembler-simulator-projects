#!/bin/bash

echo "Starting test script..."

make

for fp in tests/*.lc
do
    echo "Processing: $fp"

    base=$(basename "$fp" .lc)
    echo "Base name: $base"

    ./assembler "$fp" > output.hex

    echo "Comparing output.hex with tests/$base.hex"
    diff output.hex tests/$base.hex

    if [ $? -eq 0 ]; then
        echo "$base PASSED"
    else
        echo "$base FAILED"
    fi

done

echo "Finished."
rm -f output.hex