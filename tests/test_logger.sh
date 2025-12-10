#!/bin/bash

echo "=== 测试日志框架 ==="
echo ""

# 启动程序并让它运行几秒钟
echo "启动程序测试日志功能..."
./build/huangh-cpp &
PID=$!

# 等待程序启动
sleep 2

# 发送信号让程序正常退出
kill $PID
wait $PID 2>/dev/null

echo ""
echo "检查日志文件内容:"
echo "=== server.log 内容 ==="
cat server.log
echo "=== 结束 ==="

echo ""
echo "日志文件大小: $(stat -c%s server.log) 字节"
