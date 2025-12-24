#ifndef SQLITE_DATABASE_H
#define SQLITE_DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include "database_interface.h"
#include "config_manager.h"

class SQLiteDatabase : public DatabaseInterface
{
private:
    sqlite3 *db;
    std::string dbPath;

public:
    SQLiteDatabase(const ConfigManager &configManager);
    SQLiteDatabase(const std::string &path = "../data/students.db");
    ~SQLiteDatabase();

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

#endif // SQLITE_DATABASE_H
