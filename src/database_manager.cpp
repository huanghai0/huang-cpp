#include "database_manager.h"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::unique_ptr<DatabaseInterface> DatabaseManager::createDatabase()
{
    if (!configManager)
    {
        Logger::error("配置管理器未初始化");
        return nullptr;
    }

    std::string dbType = configManager->getDatabaseType();

    if (dbType == "sqlite")
    {
        Logger::info("使用SQLite数据库");
        return std::make_unique<SQLiteDatabase>(*configManager);
    }
    else if (dbType == "postgresql")
    {
        Logger::info("使用PostgreSQL数据库");
        return std::make_unique<PostgreSQLDatabase>(*configManager);
    }
    else
    {
        Logger::error("不支持的数据库类型: {}", dbType);
        return nullptr;
    }
}

DatabaseManager::DatabaseManager(const ConfigManager &configManager)
    : configManager(&configManager),
      redisManager(configManager.getRedisHost(), configManager.getRedisPort(), configManager.getRedisPassword())
{
    database = createDatabase();
}

DatabaseManager::DatabaseManager(const std::string &path, const std::string &redisHost, int redisPort)
    : configManager(nullptr),
      redisManager(redisHost, redisPort)
{
    // 使用默认SQLite数据库
    database = std::make_unique<SQLiteDatabase>(path);
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open()
{
    if (!database)
    {
        Logger::error("数据库实例未创建");
        return false;
    }

    // 打开数据库
    if (!database->open())
    {
        Logger::error("数据库连接失败");
        return false;
    }

    // 尝试连接Redis
    if (!redisManager.connect())
    {
        Logger::warn("Redis连接失败，将继续使用无缓存模式");
    }
    else
    {
        Logger::info("Redis连接成功");
    }

    Logger::info("数据库连接成功，类型: {}", getDatabaseType());
    return true;
}

void DatabaseManager::close()
{
    if (database)
    {
        database->close();
    }
}

std::string DatabaseManager::getDatabaseType() const
{
    if (!configManager)
    {
        return "sqlite";
    }
    return configManager->getDatabaseType();
}

int DatabaseManager::addStudent(const Student &student)
{
    if (!database)
    {
        Logger::error("数据库实例未初始化");
        return -1;
    }

    int studentId = database->addStudent(student);
    if (studentId > 0)
    {
        // 更新该学生的缓存
        updateStudentCache(studentId, student);
        Logger::info("添加学生成功，ID: {}，已更新缓存", studentId);
    }

    return studentId;
}

bool DatabaseManager::updateStudent(int id, const Student &student)
{
    if (!database)
    {
        Logger::error("数据库实例未初始化");
        return false;
    }

    bool success = database->updateStudent(id, student);
    if (success)
    {
        // 清除相关缓存
        clearStudentCache(id);
        // 更新该学生的缓存
        updateStudentCache(id, student);
        Logger::info("更新学生成功，ID: {}，已更新缓存", id);
    }

    return success;
}

bool DatabaseManager::deleteStudent(int id)
{
    if (!database)
    {
        Logger::error("数据库实例未初始化");
        return false;
    }

    bool success = database->deleteStudent(id);
    if (success)
    {
        // 清除相关缓存
        clearStudentCache(id);
        Logger::info("删除学生成功，ID: {}，已清除缓存", id);
    }

    return success;
}

Student DatabaseManager::getStudent(int id)
{
    // 尝试从缓存获取
    std::string cacheKey = "student:" + std::to_string(id);
    std::string cachedStudent = redisManager.get(cacheKey);

    if (!cachedStudent.empty())
    {
        Student student = studentFromCacheString(cachedStudent);
        if (student.getName() != "" || student.getAge() > 0 || student.getClassName() != "")
        {
            Logger::info("从缓存获取学生，ID: {}", id);
            return student;
        }
    }

    // 缓存未命中，查询数据库
    if (!database)
    {
        Logger::error("数据库实例未初始化");
        return Student();
    }

    Student student = database->getStudent(id);
    if (student.getName() != "" || student.getAge() > 0 || student.getClassName() != "")
    {
        // 写入缓存
        updateStudentCache(id, student);
        Logger::info("从数据库获取学生，ID: {}，已写入缓存", id);
    }

    return student;
}

std::vector<std::pair<int, Student>> DatabaseManager::getAllStudents()
{
    // 尝试从缓存获取
    // std::string cacheKey = "students:all";
    // std::string cachedStudents = redisManager.get(cacheKey);

    // if (!cachedStudents.empty())
    // {
    //     try
    //     {
    //         json j = json::parse(cachedStudents);
    //         std::vector<std::pair<int, Student>> students;

    //         for (const auto &item : j)
    //         {
    //             int id = item.value("id", 0);
    //             std::string name = item.value("name", "");
    //             int age = item.value("age", 0);
    //             std::string className = item.value("className", "");
    //             students.emplace_back(id, Student(name, age, className));
    //         }

    //         if (!students.empty())
    //         {
    //             Logger::info("从缓存获取所有学生，数量: {}", students.size());
    //             return students;
    //         }
    //     }
    //     catch (const json::parse_error &e)
    //     {
    //         Logger::error("缓存中学生列表解析失败: {}", e.what());
    //     }
    // }

    // 缓存未命中，查询数据库
    if (!database)
    {
        Logger::error("数据库实例未初始化");
        return {};
    }

    std::vector<std::pair<int, Student>> students = database->getAllStudents();

    // 写入缓存（即使为空也写入）
    // json j = json::array();
    // for (const auto &pair : students)
    // {
    //     json studentJson;
    //     studentJson["id"] = pair.first;
    //     studentJson["name"] = pair.second.getName();
    //     studentJson["age"] = pair.second.getAge();
    //     studentJson["className"] = pair.second.getClassName();
    //     j.push_back(studentJson);
    // }
    // redisManager.set(cacheKey, j.dump(), 60); // 1分钟过期
    Logger::info("从数据库获取所有学生，数量: {}", students.size());

    return students;
}

int DatabaseManager::getStudentCount()
{
    // 尝试从缓存获取
    std::string cacheKey = "students:count";
    std::string cachedCount = redisManager.get(cacheKey);
    if (!cachedCount.empty())
    {
        try
        {
            return std::stoi(cachedCount);
        }
        catch (const std::exception &e)
        {
            Logger::error("缓存中学生数量解析失败: {}", e.what());
        }
    }

    // 缓存未命中，查询数据库
    if (!database)
    {
        Logger::error("数据库实例未初始化");
        return -1;
    }

    int count = database->getStudentCount();
    if (count >= 0)
    {
        // 写入缓存，设置过期时间
        int expireSeconds = 30; // 默认值
        if (configManager)
        {
            expireSeconds = configManager->getCountCacheExpire();
        }
        redisManager.set(cacheKey, std::to_string(count), expireSeconds);
    }

    return count;
}

// 缓存相关方法实现
std::string DatabaseManager::studentToCacheString(const Student &student) const
{
    json j;
    j["name"] = student.getName();
    j["age"] = student.getAge();
    j["className"] = student.getClassName();
    return j.dump();
}

Student DatabaseManager::studentFromCacheString(const std::string &cacheStr) const
{
    try
    {
        json j = json::parse(cacheStr);
        std::string name = j.value("name", "");
        int age = j.value("age", 0);
        std::string className = j.value("className", "");
        return Student(name, age, className);
    }
    catch (const json::parse_error &e)
    {
        Logger::error("缓存中学生数据解析失败: {}", e.what());
        return Student();
    }
}

void DatabaseManager::clearStudentsCache()
{
    // 清除所有学生列表缓存
    // redisManager.del("students:all");
    redisManager.del("students:count");
}

void DatabaseManager::clearStudentCache(int id)
{
    std::string cacheKey = "student:" + std::to_string(id);
    redisManager.del(cacheKey);
}

void DatabaseManager::updateStudentCache(int id, const Student &student)
{
    std::string cacheKey = "student:" + std::to_string(id);
    std::string cacheValue = studentToCacheString(student);
    // 使用配置管理器获取过期时间，如果没有配置管理器则使用默认值
    int expireSeconds = 300; // 默认5分钟
    if (configManager)
    {
        expireSeconds = configManager->getStudentCacheExpire();
    }
    redisManager.set(cacheKey, cacheValue, expireSeconds);
}
