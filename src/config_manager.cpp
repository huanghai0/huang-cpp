#include "config_manager.h"
#include <fstream>
#include <iostream>
#include "logger.h"

ConfigManager::ConfigManager(const std::string &path)
    : configPath(path), loaded(false)
{
    loaded = loadConfig();
}

bool ConfigManager::loadConfig()
{
    std::ifstream configFile(configPath);
    if (!configFile.is_open())
    {
        Logger::error("无法打开配置文件: {}", configPath);
        return false;
    }

    try
    {
        configFile >> config;
        Logger::info("配置文件加载成功: {}", configPath);
        return true;
    }
    catch (const json::parse_error &e)
    {
        Logger::error("配置文件解析错误: {}", e.what());
        return false;
    }
    catch (const std::exception &e)
    {
        Logger::error("配置文件读取错误: {}", e.what());
        return false;
    }
}

std::string ConfigManager::getDatabaseType() const
{
    if (!loaded)
        return "sqlite";

    return config.value("database", json::object()).value("type", "sqlite");
}

std::string ConfigManager::getSqliteDatabasePath() const
{
    if (!loaded)
        return "../data/students.db";

    return config.value("database", json::object())
        .value("sqlite", json::object())
        .value("path", "../data/students.db");
}

std::string ConfigManager::getPostgresqlHost() const
{
    if (!loaded)
        return "192.168.2.146";

    return config.value("database", json::object())
        .value("postgresql", json::object())
        .value("host", "192.168.2.146");
}

int ConfigManager::getPostgresqlPort() const
{
    if (!loaded)
        return 5432;

    return config.value("database", json::object())
        .value("postgresql", json::object())
        .value("port", 5432);
}

std::string ConfigManager::getPostgresqlDatabase() const
{
    if (!loaded)
        return "demo1";

    return config.value("database", json::object())
        .value("postgresql", json::object())
        .value("database", "demo1");
}

std::string ConfigManager::getPostgresqlUsername() const
{
    if (!loaded)
        return "postgres";

    return config.value("database", json::object())
        .value("postgresql", json::object())
        .value("username", "postgres");
}

std::string ConfigManager::getPostgresqlPassword() const
{
    if (!loaded)
        return "deju@2025";

    return config.value("database", json::object())
        .value("postgresql", json::object())
        .value("password", "deju@2025");
}

int ConfigManager::getPostgresqlConnectionPoolSize() const
{
    if (!loaded)
        return 5;

    return config.value("database", json::object())
        .value("postgresql", json::object())
        .value("connection_pool_size", 5);
}

int ConfigManager::getPostgresqlConnectionTimeout() const
{
    if (!loaded)
        return 30;

    return config.value("database", json::object())
        .value("postgresql", json::object())
        .value("connection_timeout", 30);
}

std::string ConfigManager::getRedisHost() const
{
    if (!loaded)
        return "192.168.2.146";

    return config.value("redis", json::object()).value("host", "192.168.2.146");
}

int ConfigManager::getRedisPort() const
{
    if (!loaded)
        return 6379;

    return config.value("redis", json::object()).value("port", 6379);
}

double ConfigManager::getRedisTimeout() const
{
    if (!loaded)
        return 1.5;

    return config.value("redis", json::object()).value("timeout_seconds", 1.5);
}

std::string ConfigManager::getServerHost() const
{
    if (!loaded)
        return "localhost";

    return config.value("server", json::object()).value("host", "localhost");
}

int ConfigManager::getServerPort() const
{
    if (!loaded)
        return 8080;

    return config.value("server", json::object()).value("port", 8080);
}

int ConfigManager::getStudentCacheExpire() const
{
    if (!loaded)
        return 300;

    return config.value("cache", json::object()).value("student_expire_seconds", 300);
}

int ConfigManager::getStudentsListCacheExpire() const
{
    if (!loaded)
        return 60;

    return config.value("cache", json::object()).value("students_list_expire_seconds", 60);
}

int ConfigManager::getCountCacheExpire() const
{
    if (!loaded)
        return 30;

    return config.value("cache", json::object()).value("count_expire_seconds", 30);
}
