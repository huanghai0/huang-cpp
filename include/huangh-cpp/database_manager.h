#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>
#include "student.h"
#include "logger.h"
#include "redis_manager.h"
#include "config_manager.h"

class DatabaseManager
{
private:
    sqlite3 *db;
    std::string dbPath;
    RedisManager redisManager;
    const ConfigManager *configManager;

    // 创建学生表
    bool createStudentTable();

    // 缓存相关方法
    std::string studentToCacheString(const Student &student) const;
    Student studentFromCacheString(const std::string &cacheStr) const;
    void clearStudentsCache();
    void clearStudentCache(int id);
    void updateStudentCache(int id, const Student &student);

public:
    DatabaseManager(const ConfigManager &configManager);
    DatabaseManager(const std::string &path = "../data/students.db",
                    const std::string &redisHost = "192.168.2.146",
                    int redisPort = 6379);

    // 获取配置管理器
    const ConfigManager *getConfigManager() const { return configManager; }
    ~DatabaseManager();

    // 数据库连接管理
    bool open();
    void close();

    // 学生信息操作（带缓存）
    int addStudent(const Student &student);
    bool updateStudent(int id, const Student &student);
    bool deleteStudent(int id);
    Student getStudent(int id);
    std::vector<std::pair<int, Student>> getAllStudents();
    int getStudentCount();
};

#endif // DATABASE_MANAGER_H
