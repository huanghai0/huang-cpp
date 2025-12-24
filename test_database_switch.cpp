#include <iostream>
#include <memory>
#include "config_manager.h"
#include "database_manager.h"
#include "student.h"

int main()
{
    // 测试配置管理器
    ConfigManager configManager("config.json");

    if (!configManager.isLoaded())
    {
        std::cerr << "配置文件加载失败" << std::endl;
        return 1;
    }

    std::cout << "数据库类型: " << configManager.getDatabaseType() << std::endl;

    if (configManager.getDatabaseType() == "sqlite")
    {
        std::cout << "SQLite数据库路径: " << configManager.getSqliteDatabasePath() << std::endl;
    }
    else if (configManager.getDatabaseType() == "postgresql")
    {
        std::cout << "PostgreSQL主机: " << configManager.getPostgresqlHost() << std::endl;
        std::cout << "PostgreSQL端口: " << configManager.getPostgresqlPort() << std::endl;
        std::cout << "PostgreSQL数据库: " << configManager.getPostgresqlDatabase() << std::endl;
        std::cout << "PostgreSQL用户名: " << configManager.getPostgresqlUsername() << std::endl;
        std::cout << "PostgreSQL连接池大小: " << configManager.getPostgresqlConnectionPoolSize() << std::endl;
    }

    // 测试数据库管理器
    DatabaseManager dbManager(configManager);

    if (!dbManager.open())
    {
        std::cerr << "数据库连接失败" << std::endl;
        return 1;
    }

    std::cout << "数据库连接成功，类型: " << dbManager.getDatabaseType() << std::endl;

    // 测试添加学生
    Student student1("张三", 20, "计算机科学");
    int studentId = dbManager.addStudent(student1);

    if (studentId > 0)
    {
        std::cout << "添加学生成功，ID: " << studentId << std::endl;

        // 测试获取学生
        Student retrievedStudent = dbManager.getStudent(studentId);
        if (retrievedStudent.getName() != "")
        {
            std::cout << "获取学生成功: " << retrievedStudent.getName()
                      << ", 年龄: " << retrievedStudent.getAge()
                      << ", 班级: " << retrievedStudent.getClassName() << std::endl;
        }

        // 测试更新学生
        Student updatedStudent("李四", 21, "软件工程");
        if (dbManager.updateStudent(studentId, updatedStudent))
        {
            std::cout << "更新学生成功" << std::endl;
        }

        // 测试获取所有学生
        auto allStudents = dbManager.getAllStudents();
        std::cout << "学生总数: " << allStudents.size() << std::endl;

        // 测试学生计数
        int count = dbManager.getStudentCount();
        std::cout << "学生数量: " << count << std::endl;

        // 测试删除学生
        if (dbManager.deleteStudent(studentId))
        {
            std::cout << "删除学生成功" << std::endl;
        }
    }

    dbManager.close();
    std::cout << "测试完成" << std::endl;

    return 0;
}
