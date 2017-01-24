#!/bin/bash
src=$1
out=output
out_name=$(echo $1 | awk -F"/" '{print $2}' | awk -F"." '{print $1}')
mkdir -p $out
echo "========================================"
echo "Test: $1"
echo "========================================"
bin/parser < $src > $out/$out_name.mr
if [ $? -ne 0 ]; then
    echo -e "\e[31m========================================";
    echo "Compilation failed";
    echo -e "========================================\e[39m";
else
    echo -e "\e[32m========================================";
    echo "Running compiled program";
    echo -e "========================================\e[39m";
    bin/interpreter $out/$out_name.mr
fi
