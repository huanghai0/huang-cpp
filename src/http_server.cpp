#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "httplib.h"
#include "student.h"
#include "database_manager.h"
#include "config_manager.h"
#include "logger.h"
#include "timer.h"

using json = nlohmann::json;

// 全局配置管理器
ConfigManager configManager;

// 全局数据库管理器（使用Redis缓存）
DatabaseManager dbManager(configManager);

// 解析JSON格式的学生信息（使用nlohmann/json库）
Student parseStudentFromJson(const std::string &jsonStr)
{
    try
    {
        json j = json::parse(jsonStr);

        std::string name = j.value("name", "");
        int age = j.value("age", 0);
        std::string className = j.value("className", "");

        return Student(name, age, className);
    }
    catch (const json::parse_error &e)
    {
        Logger::error("JSON解析错误: {}", e.what());
        throw std::runtime_error("无效的JSON格式");
    }
    catch (const json::exception &e)
    {
        Logger::error("JSON处理错误: {}", e.what());
        throw std::runtime_error("JSON处理失败");
    }
}

// 将Student对象转换为JSON字符串（使用nlohmann/json库）
std::string studentToJson(const Student &student, int id = -1)
{
    json j;

    if (id != -1)
    {
        j["id"] = id;
    }

    j["name"] = student.getName();
    j["age"] = student.getAge();
    j["className"] = student.getClassName();

    return j.dump();
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

    // 使用配置中的服务器设置
    std::string serverHost = configManager.getServerHost();
    int serverPort = configManager.getServerPort();

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
                json errorJson;
                errorJson["error"] = "数据库操作失败";
                res.set_content(errorJson.dump(), "application/json");
                Logger::error("添加学生失败");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            json errorJson;
            errorJson["error"] = "无效的学生数据";
            res.set_content(errorJson.dump(), "application/json");
            Logger::error("添加学生失败: {}", e.what());
        } });

    // 获取所有学生信息 - GET /students
    svr.Get("/students", [](const httplib::Request &req, httplib::Response &res)
            {
        Timer timer;
        Logger::info("收到获取所有学生请求");
        
        auto students = dbManager.getAllStudents();
        json j = json::array();
        
        for (const auto& pair : students) {
            json studentJson;
            studentJson["id"] = pair.first;
            studentJson["name"] = pair.second.getName();
            studentJson["age"] = pair.second.getAge();
            studentJson["className"] = pair.second.getClassName();
            j.push_back(studentJson);
        }
        
        res.set_content(j.dump(), "application/json");
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
            json errorJson;
            errorJson["error"] = "学生不存在";
            res.set_content(errorJson.dump(), "application/json");
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
                json errorJson;
                errorJson["error"] = "学生不存在";
                res.set_content(errorJson.dump(), "application/json");
                Logger::warn("学生不存在，ID: {}", studentId);
            }
        } catch (const std::exception& e) {
            res.status = 400;
            json errorJson;
            errorJson["error"] = "无效的学生数据";
            res.set_content(errorJson.dump(), "application/json");
            Logger::error("更新学生失败: {}", e.what());
        } });

    // 删除学生信息 - DELETE /students/{id}
    svr.Delete(R"(/students/(\d+))", [](const httplib::Request &req, httplib::Response &res)
               {
        int studentId = std::stoi(req.matches[1]);
        Logger::info("收到删除学生请求，ID: {}", studentId);
        
        bool success = dbManager.deleteStudent(studentId);
        if (success) {
            json successJson;
            successJson["message"] = "学生删除成功";
            res.set_content(successJson.dump(), "application/json");
            Logger::info("成功删除学生");
        } else {
            res.status = 404;
            json errorJson;
            errorJson["error"] = "学生不存在";
            res.set_content(errorJson.dump(), "application/json");
            Logger::warn("学生不存在，ID: {}", studentId);
        } });

    // 健康检查接口
    svr.Get("/health", [](const httplib::Request &req, httplib::Response &res)
            { 
        int count = dbManager.getStudentCount();
        json j;
        
        if (count >= 0) {
            j["status"] = "ok";
            j["students_count"] = count;
            res.set_content(j.dump(), "application/json");
        } else {
            res.status = 500;
            j["error"] = "数据库查询失败";
            res.set_content(j.dump(), "application/json");
        } });

    Logger::info("HTTP服务器启动在 http://localhost:8080");
    Logger::info("可用接口:");
    Logger::info("  POST   /students     - 添加学生");
    Logger::info("  GET    /students     - 获取所有学生");
    Logger::info("  GET    /students/{{id}} - 获取特定学生");
    Logger::info("  PUT    /students/{{id}} - 更新学生");
    Logger::info("  DELETE /students/{{id}} - 删除学生");
    Logger::info("  GET    /health       - 健康检查");

    Logger::info("开始监听端口 {}...", serverPort);
    svr.listen(serverHost.c_str(), serverPort);
}
