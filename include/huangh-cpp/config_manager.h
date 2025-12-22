#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ConfigManager
{
private:
    json config;
    std::string configPath;
    bool loaded;

    bool loadConfig();

public:
    ConfigManager(const std::string &path = "config.json");

    // 数据库配置
    std::string getDatabasePath() const;
    std::string getDatabaseType() const;

    // Redis配置
    std::string getRedisHost() const;
    int getRedisPort() const;
    double getRedisTimeout() const;

    // 服务器配置
    std::string getServerHost() const;
    int getServerPort() const;

    // 缓存配置
    int getStudentCacheExpire() const;
    int getStudentsListCacheExpire() const;
    int getCountCacheExpire() const;

    // 检查配置是否加载成功
    bool isLoaded() const { return loaded; }

    // 获取原始JSON配置
    const json &getConfig() const { return config; }
};

#endif // CONFIG_MANAGER_H
