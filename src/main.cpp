#include <iostream>
#include <nlohmann/json.hpp>
#include "logger.h"

using json = nlohmann::json;

// 声明集合示例函数
void vector_example();
void list_example();
void map_example();
void set_example();
void unordered_collections_example();
void student_collections_example();

// 声明HTTP服务器函数
void startHttpServer();

int main(int argc, char *argv[])
{
    // 初始化日志系统
    if (!Logger::initialize("server.log"))
    {
        std::cerr << "Failed to initialize logger" << std::endl;
        return -1;
    }

    Logger::info("Hello, World! This is huangh-cpp project.");

    std::unordered_map<const char *, double> c_umap{{"one", 1.2}, {"two", 2.3}, {"three", 3.4}};
    json j_umap(c_umap);
    Logger::info("JSON from unordered_map: {}", j_umap.dump());

    // 选择运行模式
    std::cout << "\n请选择运行模式:" << std::endl;
    std::cout << "1. 运行集合示例" << std::endl;
    std::cout << "2. 启动HTTP服务器" << std::endl;
    std::cout << "请输入选择 (1 或 2): ";

    int choice = argc > 1 ? std::stoi(argv[1]) : 0;

    if (choice == 1)
    {
        std::cout << "\n=== C++ STL Collections Examples ===" << std::endl;
        // 调用各种集合示例
        vector_example();
        list_example();
        map_example();
        set_example();
        unordered_collections_example();
        student_collections_example();
        std::cout << "\n=== All examples completed ===" << std::endl;
    }
    else if (choice == 2)
    {
        std::cout << "\n=== 启动HTTP服务器 ===" << std::endl;
        startHttpServer();
    }
    else
    {
        std::cout << "无效的选择，退出程序。" << std::endl;
    }

    return 0;
}
