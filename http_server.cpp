#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include "httplib.h"
#include "student.h"
#include "database_manager.h"
#include "logger.h"

// 全局数据库管理器
DatabaseManager dbManager;

// 解析JSON格式的学生信息（简化版）
Student parseStudentFromJson(const std::string &jsonStr)
{
    std::string name = "";
    int age = 0;
    std::string className = "";

    // 简单的JSON解析（实际项目中应该使用JSON库）
    size_t namePos = jsonStr.find("\"name\":\"");
    if (namePos != std::string::npos)
    {
        size_t nameStart = namePos + 8;
        size_t nameEnd = jsonStr.find("\"", nameStart);
        if (nameEnd != std::string::npos)
        {
            name = jsonStr.substr(nameStart, nameEnd - nameStart);
        }
    }

    size_t agePos = jsonStr.find("\"age\":");
    if (agePos != std::string::npos)
    {
        size_t ageStart = agePos + 6;
        size_t ageEnd = jsonStr.find_first_of(",}", ageStart);
        if (ageEnd != std::string::npos)
        {
            std::string ageStr = jsonStr.substr(ageStart, ageEnd - ageStart);
            age = std::stoi(ageStr);
        }
    }

    size_t classPos = jsonStr.find("\"className\":\"");
    if (classPos != std::string::npos)
    {
        size_t classStart = classPos + 13;
        size_t classEnd = jsonStr.find("\"", classStart);
        if (classEnd != std::string::npos)
        {
            className = jsonStr.substr(classStart, classEnd - classStart);
        }
    }

    return Student(name, age, className);
}

// 将Student对象转换为JSON字符串
std::string studentToJson(const Student &student, int id = -1)
{
    std::stringstream ss;
    ss << "{";
    if (id != -1)
    {
        ss << "\"id\":" << id << ",";
    }
    ss << "\"name\":\"" << student.getName() << "\",";
    ss << "\"age\":" << student.getAge() << ",";
    ss << "\"className\":\"" << student.getClassName() << "\"";
    ss << "}";
    return ss.str();
}

// 启动HTTP服务器
void startHttpServer()
{
    // 打开数据库连接
    if (!dbManager.open())
    {
        Logger::error("数据库连接失败，无法启动服务器");
        return;
    }

    httplib::Server svr;

    // 添加学生信息 - POST /students
    svr.Post("/students", [](const httplib::Request &req, httplib::Response &res)
             {
        Logger::info("收到添加学生请求: {}", req.body);
        
        try {
            Student student = parseStudentFromJson(req.body);
            int studentId = dbManager.addStudent(student);
            
            if (studentId > 0) {
                std::string response = studentToJson(student, studentId);
                res.set_content(response, "application/json");
                Logger::info("成功添加学生，ID: {}", studentId);
            } else {
                res.status = 500;
                res.set_content("{\"error\":\"数据库操作失败\"}", "application/json");
                Logger::error("添加学生失败");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content("{\"error\":\"无效的学生数据\"}", "application/json");
            Logger::error("添加学生失败: {}", e.what());
        } });

    // 获取所有学生信息 - GET /students
    svr.Get("/students", [](const httplib::Request &req, httplib::Response &res)
            {
        Logger::info("收到获取所有学生请求");
        
        auto students = dbManager.getAllStudents();
        std::stringstream ss;
        ss << "[";
        bool first = true;
        for (const auto& pair : students) {
            if (!first) {
                ss << ",";
            }
            ss << studentToJson(pair.second, pair.first);
            first = false;
        }
        ss << "]";
        
        res.set_content(ss.str(), "application/json");
        Logger::info("返回 {} 个学生信息", students.size()); });

    // 获取特定学生信息 - GET /students/{id}
    svr.Get(R"(/students/(\d+))", [](const httplib::Request &req, httplib::Response &res)
            {
        int studentId = std::stoi(req.matches[1]);
        Logger::info("收到获取学生请求，ID: {}", studentId);
        
        Student student = dbManager.getStudent(studentId);
        // 检查学生是否存在，确保所有字段都有有效值
        if (student.getName() != "" && student.getAge() > 0 && student.getClassName() != "") {
            std::string response = studentToJson(student, studentId);
            res.set_content(response, "application/json");
            Logger::info("成功返回学生信息");
        } else {
            res.status = 404;
            res.set_content("{\"error\":\"学生不存在\"}", "application/json");
            Logger::warn("学生不存在，ID: {}", studentId);
        } });

    // 更新学生信息 - PUT /students/{id}
    svr.Put(R"(/students/(\d+))", [](const httplib::Request &req, httplib::Response &res)
            {
        int studentId = std::stoi(req.matches[1]);
        Logger::info("收到更新学生请求，ID: {} 数据: {}", studentId, req.body);
        
        try {
            Student student = parseStudentFromJson(req.body);
            bool success = dbManager.updateStudent(studentId, student);
            
            if (success) {
                std::string response = studentToJson(student, studentId);
                res.set_content(response, "application/json");
                Logger::info("成功更新学生信息");
            } else {
                res.status = 404;
                res.set_content("{\"error\":\"学生不存在\"}", "application/json");
                Logger::warn("学生不存在，ID: {}", studentId);
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content("{\"error\":\"无效的学生数据\"}", "application/json");
            Logger::error("更新学生失败: {}", e.what());
        } });

    // 删除学生信息 - DELETE /students/{id}
    svr.Delete(R"(/students/(\d+))", [](const httplib::Request &req, httplib::Response &res)
               {
        int studentId = std::stoi(req.matches[1]);
        Logger::info("收到删除学生请求，ID: {}", studentId);
        
        bool success = dbManager.deleteStudent(studentId);
        if (success) {
            res.set_content("{\"message\":\"学生删除成功\"}", "application/json");
            Logger::info("成功删除学生");
        } else {
            res.status = 404;
            res.set_content("{\"error\":\"学生不存在\"}", "application/json");
            Logger::warn("学生不存在，ID: {}", studentId);
        } });

    // 健康检查接口
    svr.Get("/health", [](const httplib::Request &req, httplib::Response &res)
            { 
        int count = dbManager.getStudentCount();
        if (count >= 0) {
            res.set_content("{\"status\":\"ok\",\"students_count\":" + std::to_string(count) + "}", "application/json");
        } else {
            res.status = 500;
            res.set_content("{\"error\":\"数据库查询失败\"}", "application/json");
        } });

    Logger::info("HTTP服务器启动在 http://localhost:8080");
    Logger::info("可用接口:");
    Logger::info("  POST   /students     - 添加学生");
    Logger::info("  GET    /students     - 获取所有学生");
    Logger::info("  GET    /students/{{id}} - 获取特定学生");
    Logger::info("  PUT    /students/{{id}} - 更新学生");
    Logger::info("  DELETE /students/{{id}} - 删除学生");
    Logger::info("  GET    /health       - 健康检查");

    Logger::info("开始监听端口 8080...");
    svr.listen("localhost", 8080);
}
