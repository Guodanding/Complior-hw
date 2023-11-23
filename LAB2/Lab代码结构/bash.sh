#!/bin/bash

# 循环遍历文件并执行操作
for ((i=1; i<=17; i++))
do
    input_file="./Test/test${i}.cmm"
    output_file="./Output/test${i}.cmm"
    ./parser "$input_file" > "$output_file"
    echo "Processed $input_file"
done

for ((i=1; i<=6; i++))
do
    input_file="./Test/test_o${i}.cmm"
    output_file="./Output/test_o${i}.cmm"
    ./parser "$input_file" > "$output_file"
    echo "Processed $input_file"
done

./parser ./Test/self_test.cmm > ./Output/self_test.cmm
echo "Processed self_test.cmm"

./parser ./Test/in/testcase_1 > ./Output/testcase_1
echo "Processed testcase_1"

./parser ./Test/in/testcase_11 > ./Output/testcase_11
echo "Processed testcase_11"

./parser ./Test/in/testcase_21 > ./Output/testcase_21
echo "Processed testcase_21"
