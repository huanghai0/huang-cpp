#!/bin/bash

echo "=== HTTP服务器测试脚本 (SQLite版本) ==="
echo "注意：请先运行程序并选择模式2启动HTTP服务器"
echo "数据将持久化存储在 students.db 文件中"
echo ""

# 等待用户启动服务器
read -p "按回车键继续测试..."

echo ""
echo "1. 测试健康检查接口"
curl -s http://localhost:8080/health | python3 -m json.tool

echo ""
echo "2. 添加学生信息"
curl -X POST http://localhost:8080/students \
  -H "Content-Type: application/json" \
  -d '{"name":"张三","age":20,"className":"计算机科学1班"}' | python3 -m json.tool

echo ""
echo "3. 添加第二个学生"
curl -X POST http://localhost:8080/students \
  -H "Content-Type: application/json" \
  -d '{"name":"李四","age":22,"className":"软件工程2班"}' | python3 -m json.tool

echo ""
echo "4. 获取所有学生信息"
curl -s http://localhost:8080/students | python3 -m json.tool

echo ""
echo "5. 获取特定学生信息 (ID=1)"
curl -s http://localhost:8080/students/1 | python3 -m json.tool

echo ""
echo "6. 更新学生信息 (ID=1)"
curl -X PUT http://localhost:8080/students/1 \
  -H "Content-Type: application/json" \
  -d '{"name":"张三丰","age":21,"className":"计算机科学1班"}' | python3 -m json.tool

echo ""
echo "7. 再次获取所有学生信息"
curl -s http://localhost:8080/students | python3 -m json.tool

echo ""
echo "8. 删除学生信息 (ID=2)"
curl -X DELETE http://localhost:8080/students/2 | python3 -m json.tool

echo ""
echo "9. 最终学生列表"
curl -s http://localhost:8080/students | python3 -m json.tool

echo ""
echo "10. 查看数据库文件"
if [ -f "students.db" ]; then
    echo "数据库文件 students.db 已创建"
    sqlite3 students.db "SELECT * FROM students;" 2>/dev/null || echo "无法查询数据库，可能需要sqlite3命令行工具"
else
    echo "数据库文件 students.db 不存在"
fi

echo ""
echo "=== 测试完成 ==="
echo "提示：数据已持久化存储在 students.db 文件中，重启服务器后数据不会丢失"
