#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>
#include "student.h"
#include "logger.h"

class DatabaseManager
{
private:
    sqlite3 *db;
    std::string dbPath;

    // 创建学生表
    bool createStudentTable();

public:
    DatabaseManager(const std::string &path = "students.db");
    ~DatabaseManager();

    // 数据库连接管理
    bool open();
    void close();

    // 学生信息操作
    int addStudent(const Student &student);
    bool updateStudent(int id, const Student &student);
    bool deleteStudent(int id);
    Student getStudent(int id);
    std::vector<std::pair<int, Student>> getAllStudents();
    int getStudentCount();
};

#endif // DATABASE_MANAGER_H
