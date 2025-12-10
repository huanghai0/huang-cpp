#include "database_manager.h"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

DatabaseManager::DatabaseManager(const std::string &path, const std::string &redisHost, int redisPort)
    : db(nullptr), dbPath(path), redisManager(redisHost, redisPort)
{
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open()
{
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK)
    {
        Logger::error("无法打开数据库: {}", sqlite3_errmsg(db));
        return false;
    }

    // 创建学生表
    if (!createStudentTable())
    {
        Logger::error("创建学生表失败");
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

    Logger::info("数据库连接成功: {}", dbPath);
    return true;
}

void DatabaseManager::close()
{
    if (db)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool DatabaseManager::createStudentTable()
{
    const char *sql = "CREATE TABLE IF NOT EXISTS students ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "name TEXT NOT NULL,"
                      "age INTEGER NOT NULL,"
                      "className TEXT NOT NULL);";

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        Logger::error("SQL错误: {}", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

int DatabaseManager::addStudent(const Student &student)
{
    const char *sql = "INSERT INTO students (name, age, className) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, student.getName().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, student.getAge());
    sqlite3_bind_text(stmt, 3, student.getClassName().c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        Logger::error("执行SQL语句失败: {}", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    int studentId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);

    // 清除所有学生列表缓存，因为列表已变化
    // clearStudentsCache();

    // 更新该学生的缓存
    updateStudentCache(studentId, student);

    Logger::info("添加学生成功，ID: {}，已更新缓存", studentId);
    return studentId;
}

bool DatabaseManager::updateStudent(int id, const Student &student)
{
    const char *sql = "UPDATE students SET name = ?, age = ?, className = ? WHERE id = ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, student.getName().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, student.getAge());
    sqlite3_bind_text(stmt, 3, student.getClassName().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        Logger::error("执行SQL语句失败: {}", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    bool success = sqlite3_changes(db) > 0;
    sqlite3_finalize(stmt);

    if (success)
    {
        // 清除相关缓存
        clearStudentCache(id);
        // clearStudentsCache();
        // 更新该学生的缓存
        updateStudentCache(id, student);
        Logger::info("更新学生成功，ID: {}，已更新缓存", id);
    }

    return success;
}

bool DatabaseManager::deleteStudent(int id)
{
    const char *sql = "DELETE FROM students WHERE id = ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        Logger::error("执行SQL语句失败: {}", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    bool success = sqlite3_changes(db) > 0;
    sqlite3_finalize(stmt);

    if (success)
    {
        // 清除相关缓存
        clearStudentCache(id);
        // clearStudentsCache();
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
    const char *sql = "SELECT name, age, className FROM students WHERE id = ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return Student();
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        int age = sqlite3_column_int(stmt, 1);
        std::string className = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        sqlite3_finalize(stmt);

        Student student(name, age, className);
        // 写入缓存
        updateStudentCache(id, student);
        Logger::info("从数据库获取学生，ID: {}，已写入缓存", id);
        return student;
    }

    sqlite3_finalize(stmt);
    return Student();
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
    std::vector<std::pair<int, Student>> students;
    const char *sql = "SELECT id, name, age, className FROM students;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return students;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        int age = sqlite3_column_int(stmt, 2);
        std::string className = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        students.emplace_back(id, Student(name, age, className));
    }

    sqlite3_finalize(stmt);

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
    Logger::info("从数据库获取所有学生，数量: {}，已写入缓存", students.size());

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
    const char *sql = "SELECT COUNT(*) FROM students;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        // 写入缓存，设置30秒过期时间
        redisManager.set(cacheKey, std::to_string(count), 30);
        return count;
    }

    sqlite3_finalize(stmt);
    return -1;
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
    redisManager.set(cacheKey, cacheValue, 300); // 5分钟过期
}
