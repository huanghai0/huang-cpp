#include <iostream>
#include "config_manager.h"
#include "database_manager.h"
#include "student.h"

int main()
{
    std::cout << "=== 测试 PostgreSQL 数据库管理器 ===" << std::endl;

    // 加载 PostgreSQL 配置
    ConfigManager config("config_postgresql.json");
    if (!config.isLoaded())
    {
        std::cerr << "配置文件加载失败" << std::endl;
        return 1;
    }

    std::cout << "数据库类型: " << config.getDatabaseType() << std::endl;

    // 创建数据库管理器
    DatabaseManager dbManager(config);

    if (!dbManager.open())
    {
        std::cerr << "数据库连接失败" << std::endl;
        return 1;
    }

    std::cout << "数据库连接成功，类型: " << dbManager.getDatabaseType() << std::endl;

    // 测试添加学生
    Student student("PostgreSQL测试学生", 22, "计算机科学");
    int id = dbManager.addStudent(student);

    if (id > 0)
    {
        std::cout << "添加学生成功，ID: " << id << std::endl;

        // 测试获取学生
        Student s = dbManager.getStudent(id);
        if (s.getName() != "")
        {
            std::cout << "获取学生: " << s.getName()
                      << ", 年龄: " << s.getAge()
                      << ", 班级: " << s.getClassName() << std::endl;
        }

        // 测试更新学生
        Student updatedStudent("更新后的学生", 23, "软件工程");
        if (dbManager.updateStudent(id, updatedStudent))
        {
            std::cout << "更新学生成功" << std::endl;
        }

        // 测试获取所有学生
        auto allStudents = dbManager.getAllStudents();
        std::cout << "学生总数: " << allStudents.size() << std::endl;

        // 测试学生计数
        int count = dbManager.getStudentCount();
        std::cout << "学生数量: " << count << std::endl;

        // 清理测试数据
        if (dbManager.deleteStudent(id))
        {
            std::cout << "删除测试学生成功" << std::endl;
        }
    }
    else
    {
        std::cerr << "添加学生失败" << std::endl;
    }

    dbManager.close();
    std::cout << "=== PostgreSQL 数据库管理器测试完成 ===" << std::endl;

    return 0;
}
