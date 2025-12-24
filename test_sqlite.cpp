#include <iostream>
#include "config_manager.h"
#include "database_manager.h"
#include "student.h"

int main()
{
    std::cout << "=== 测试 SQLite 数据库 ===" << std::endl;

    // 加载配置
    ConfigManager config("config.json");
    if (!config.isLoaded())
    {
        std::cerr << "配置文件加载失败" << std::endl;
        return 1;
    }

    std::cout << "数据库类型: " << config.getDatabaseType() << std::endl;
    std::cout << "SQLite路径: " << config.getSqliteDatabasePath() << std::endl;

    // 创建数据库管理器
    DatabaseManager dbManager(config);

    if (!dbManager.open())
    {
        std::cerr << "数据库连接失败" << std::endl;
        return 1;
    }

    std::cout << "数据库连接成功" << std::endl;

    // 测试添加学生
    Student student("测试学生", 25, "测试班级");
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

        // 测试计数
        int count = dbManager.getStudentCount();
        std::cout << "学生总数: " << count << std::endl;

        // 清理测试数据
        if (dbManager.deleteStudent(id))
        {
            std::cout << "删除测试学生成功" << std::endl;
        }
    }

    dbManager.close();
    std::cout << "=== SQLite 测试完成 ===" << std::endl;
    return 0;
}
