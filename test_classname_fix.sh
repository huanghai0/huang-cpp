#!/bin/bash

echo "=== 测试className字段修复 ==="
echo "注意：请先运行程序并选择模式2启动HTTP服务器"
echo ""

# 等待用户启动服务器
read -p "按回车键继续测试..."

echo ""
echo "1. 添加学生信息（包含className）"
curl -X POST http://localhost:8080/students \
  -H "Content-Type: application/json" \
  -d '{"name":"测试学生","age":18,"className":"测试班级"}' | python3 -m json.tool

echo ""
echo "2. 获取所有学生信息"
curl -s http://localhost:8080/students | python3 -m json.tool

echo ""
echo "3. 获取特定学生信息 (ID=1)"
curl -s http://localhost:8080/students/1 | python3 -m json.tool

echo ""
echo "4. 检查className字段是否为空"
response=$(curl -s http://localhost:8080/students/1)
echo "响应: $response"

# 检查className字段
if echo "$response" | grep -q '"className":"测试班级"'; then
    echo "✅ className字段正确显示: 测试班级"
else
    echo "❌ className字段显示有问题"
fi

echo ""
echo "=== 测试完成 ==="
