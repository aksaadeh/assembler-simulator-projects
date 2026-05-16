#!/bin/bash

make

run_test() {
    local filename="$1"

    ./simulator $2 $3 $4 < "tests/$filename.in" > "tests/$filename.actual"

    if diff "tests/$filename.actual" "tests/$filename.out" > /dev/null; then
        echo "Test $filename passed!"
    else
        echo "Test $filename failed!"
    fi
}

run_test "1" 4 1 1
run_test "2" 8 2 2
run_test "3" 4 2 1
run_test "4" 12 2 1
run_test "5" 14 2 2
run_test "6"
run_test "6" 4
run_test "6" 4 1
run_test "6" abc 1 1
run_test "6" 4 0 1
run_test "6" 4 -1 1
run_test "7" 8 2 1
run_test "8" 4 1 1
run_test "9" 4 1 1
run_test "10" 14 2 20
run_test "11" 12 2 2
run_test "12" 12 1 1