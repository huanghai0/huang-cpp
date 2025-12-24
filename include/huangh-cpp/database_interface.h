#ifndef DATABASE_INTERFACE_H
#define DATABASE_INTERFACE_H

#include <string>
#include <vector>
#include "student.h"

class DatabaseInterface
{
public:
    virtual ~DatabaseInterface() = default;

    // 数据库连接管理
    virtual bool open() = 0;
    virtual void close() = 0;

    // 学生信息操作
    virtual int addStudent(const Student &student) = 0;
    virtual bool updateStudent(int id, const Student &student) = 0;
    virtual bool deleteStudent(int id) = 0;
    virtual Student getStudent(int id) = 0;
    virtual std::vector<std::pair<int, Student>> getAllStudents() = 0;
    virtual int getStudentCount() = 0;

    // 表创建
    virtual bool createStudentTable() = 0;
};

#endif // DATABASE_INTERFACE_H
