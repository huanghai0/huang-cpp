#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include "student.h"
#include "logger.h"
#include "redis_manager.h"
#include "config_manager.h"
#include "database_interface.h"
#include "sqlite_database.h"
#include "postgresql_database.h"

class DatabaseManager
{
private:
    std::unique_ptr<DatabaseInterface> database;
    RedisManager redisManager;
    const ConfigManager *configManager;

    // 缓存相关方法
    std::string studentToCacheString(const Student &student) const;
    Student studentFromCacheString(const std::string &cacheStr) const;
    void clearStudentsCache();
    void clearStudentCache(int id);
    void updateStudentCache(int id, const Student &student);

    // 根据配置创建数据库实例
    std::unique_ptr<DatabaseInterface> createDatabase();

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

    // 获取当前数据库类型
    std::string getDatabaseType() const;
};

#endif // DATABASE_MANAGER_H
