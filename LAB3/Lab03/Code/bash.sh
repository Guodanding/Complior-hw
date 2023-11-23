#!/bin/bash

input_dir="../Test"
output_dir="../result"

# 检查输出目录是否存在，不存在则创建
mkdir -p "$output_dir"

# 循环遍历输入目录中的所有文件
for input_file in "$input_dir"/*; do
    # 提取文件名（不包含路径）
    file_name=$(basename "$input_file")

    # 构建输出文件的完整路径，将后缀改为 .ir
    output_file="$output_dir/${file_name%.*}.ir"

    # 运行 ./c 程序，将输入文件的内容写入输出文件
    ./parser "$input_file" "$output_file"

    # 打印处理信息（可选）
    echo "Processed: $input_file -> $output_file"
done
