#!/bin/bash
src=$1
out_name=${1:5:-4}
mkdir -p test_out
echo "========================================"
echo "Test: $1"
echo "========================================"
bin/parser < $src > test_out/$out_name.mr
echo "========================================"
echo "Running compiled program"
echo "========================================"
bin/interpreter test_out/$out_name.mr