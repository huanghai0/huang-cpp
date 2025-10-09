# HTTP服务器 - 学生信息管理系统

这是一个基于C++和httplib库构建的HTTP服务器，用于管理学生信息。

## 功能特性

- ✅ 添加学生信息 (POST /students)
- ✅ 获取所有学生信息 (GET /students)
- ✅ 获取特定学生信息 (GET /students/{id})
- ✅ 更新学生信息 (PUT /students/{id})
- ✅ 删除学生信息 (DELETE /students/{id})
- ✅ 健康检查接口 (GET /health)

## 快速开始

### 1. 编译项目

```bash
cd build
make
```

### 2. 运行程序

```bash
./bin/huangh-cpp
```

在程序启动后，选择模式2启动HTTP服务器：
```
请选择运行模式:
1. 运行集合示例
2. 启动HTTP服务器
请输入选择 (1 或 2): 2
```

### 3. 测试API

服务器启动后，可以使用以下方式测试：

#### 方法1: 使用测试脚本
```bash
./test_http_server.sh
```

#### 方法2: 手动测试

**添加学生信息:**
```bash
curl -X POST http://localhost:8080/students \
  -H "Content-Type: application/json" \
  -d '{"name":"张三","age":20,"className":"计算机科学1班"}'
```

**获取所有学生信息:**
```bash
curl http://localhost:8080/students
```

**获取特定学生信息:**
```bash
curl http://localhost:8080/students/1
```

**更新学生信息:**
```bash
curl -X PUT http://localhost:8080/students/1 \
  -H "Content-Type: application/json" \
  -d '{"name":"张三丰","age":21,"className":"计算机科学1班"}'
```

**删除学生信息:**
```bash
curl -X DELETE http://localhost:8080/students/1
```

**健康检查:**
```bash
curl http://localhost:8080/health
```

## API接口文档

### POST /students
添加新的学生信息

**请求体:**
```json
{
  "name": "学生姓名",
  "age": 年龄,
  "className": "班级名称"
}
```

**响应:**
```json
{
  "id": 1,
  "name": "学生姓名",
  "age": 年龄,
  "className": "班级名称"
}
```

### GET /students
获取所有学生信息

**响应:**
```json
[
  {
    "id": 1,
    "name": "学生姓名",
    "age": 年龄,
    "className": "班级名称"
  }
]
```

### GET /students/{id}
获取特定学生信息

**参数:**
- id: 学生ID

**响应:**
```json
{
  "id": 1,
  "name": "学生姓名",
  "age": 年龄,
  "className": "班级名称"
}
```

### PUT /students/{id}
更新学生信息

**参数:**
- id: 学生ID

**请求体:**
```json
{
  "name": "新的学生姓名",
  "age": 新的年龄,
  "className": "新的班级名称"
}
```

### DELETE /students/{id}
删除学生信息

**参数:**
- id: 学生ID

**响应:**
```json
{
  "message": "学生删除成功"
}
```

### GET /health
健康检查接口

**响应:**
```json
{
  "status": "ok",
  "students_count": 当前学生数量
}
```

## 技术实现

- **语言**: C++17
- **HTTP库**: cpp-httplib (单头文件库)
- **数据库**: SQLite3 (轻量级嵌入式数据库)
- **数据存储**: students.db 文件
- **JSON处理**: 简易字符串解析 (生产环境建议使用nlohmann/json等库)

## 注意事项

1. 服务器运行在 `http://localhost:8080`
2. 学生信息持久化存储在 `students.db` SQLite数据库中，重启服务器后数据不会丢失
3. 当前使用简易JSON解析，建议在生产环境中使用成熟的JSON库
4. 服务器是单线程的，适合学习和测试使用
5. 数据库文件 `students.db` 会在首次运行时自动创建
