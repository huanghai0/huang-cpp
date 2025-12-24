#ifndef POSTGRESQL_DATABASE_H
#define POSTGRESQL_DATABASE_H

#include <libpq-fe.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "database_interface.h"
#include "config_manager.h"

class PostgreSQLDatabase : public DatabaseInterface
{
private:
    // 连接池
    struct ConnectionPool
    {
        std::vector<PGconn *> connections;
        std::mutex mutex;
        size_t maxSize;
        std::string connectionString;
    };

    std::shared_ptr<ConnectionPool> connectionPool;
    const ConfigManager *configManager;

    // 获取连接
    PGconn *acquireConnection();

    // 释放连接
    void releaseConnection(PGconn *conn);

    // 创建连接池
    bool createConnectionPool();

    // 执行查询
    PGresult *executeQuery(PGconn *conn, const std::string &sql, const std::vector<std::string> &params = {});

public:
    PostgreSQLDatabase(const ConfigManager &configManager);
    ~PostgreSQLDatabase();

    // 数据库连接管理
    bool open() override;
    void close() override;

    // 学生信息操作
    int addStudent(const Student &student) override;
    bool updateStudent(int id, const Student &student) override;
    bool deleteStudent(int id) override;
    Student getStudent(int id) override;
    std::vector<std::pair<int, Student>> getAllStudents() override;
    int getStudentCount() override;

    // 表创建
    bool createStudentTable() override;
};

#endif // POSTGRESQL_DATABASE_H
