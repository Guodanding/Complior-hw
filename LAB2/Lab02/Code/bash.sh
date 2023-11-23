#!/bin/bash

# 循环遍历文件并执行操作
for ((i=1; i<=17; i++))
do
    input_file="../Test/test${i}.cmm"
    output_file="../Output/test${i}.cmm"
    ./parser "$input_file" > "$output_file"
    echo "Processed $input_file"
done

for ((i=1; i<=6; i++))
do
    input_file="../Test/test_o${i}.cmm"
    output_file="../Output/test_o${i}.cmm"
    ./parser "$input_file" > "$output_file"
    echo "Processed $input_file"
done

do
./parser ../Test/self_test.cmm > ../Output/self_test.cmm
echo "Processed self_test.cmm"

./parser ../Test/in/test_case1 > ../Output/test_case1
echo "Processed test_case1"

./parser ../Test/in/test_case11 > ../Output/test_case11
echo "Processed test_case11"

./parser ../Test/in/test_case21 > ../Output/test_case21
echo "Processed test_case21"
done

