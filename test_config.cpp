#include <iostream>
#include "config_manager.h"

int main()
{
    ConfigManager configManager;

    if (!configManager.isLoaded())
    {
        std::cout << "配置文件加载失败！" << std::endl;
        return 1;
    }

    std::cout << "=== 配置测试 ===" << std::endl;
    std::cout << "数据库路径: " << configManager.getDatabasePath() << std::endl;
    std::cout << "数据库类型: " << configManager.getDatabaseType() << std::endl;
    std::cout << "Redis主机: " << configManager.getRedisHost() << std::endl;
    std::cout << "Redis端口: " << configManager.getRedisPort() << std::endl;
    std::cout << "Redis超时: " << configManager.getRedisTimeout() << "秒" << std::endl;
    std::cout << "服务器主机: " << configManager.getServerHost() << std::endl;
    std::cout << "服务器端口: " << configManager.getServerPort() << std::endl;
    std::cout << "学生缓存过期: " << configManager.getStudentCacheExpire() << "秒" << std::endl;
    std::cout << "学生列表缓存过期: " << configManager.getStudentsListCacheExpire() << "秒" << std::endl;
    std::cout << "计数缓存过期: " << configManager.getCountCacheExpire() << "秒" << std::endl;

    return 0;
}
