#include <iostream>
#include <libpq-fe.h>
#include "config_manager.h"

int main()
{
    std::cout << "=== 测试 PostgreSQL 连接 ===" << std::endl;

    // 加载配置
    ConfigManager config("config_postgresql.json");
    if (!config.isLoaded())
    {
        std::cerr << "配置文件加载失败" << std::endl;
        return 1;
    }

    std::cout << "数据库类型: " << config.getDatabaseType() << std::endl;
    std::cout << "PostgreSQL主机: " << config.getPostgresqlHost() << std::endl;
    std::cout << "PostgreSQL端口: " << config.getPostgresqlPort() << std::endl;
    std::cout << "PostgreSQL数据库: " << config.getPostgresqlDatabase() << std::endl;
    std::cout << "PostgreSQL用户名: " << config.getPostgresqlUsername() << std::endl;

    // 测试直接连接
    std::string connStr = "host=" + config.getPostgresqlHost() +
                          " port=" + std::to_string(config.getPostgresqlPort()) +
                          " dbname=" + config.getPostgresqlDatabase() +
                          " user=" + config.getPostgresqlUsername() +
                          " password=" + config.getPostgresqlPassword() +
                          " connect_timeout=" + std::to_string(config.getPostgresqlConnectionTimeout());

    std::cout << "连接字符串: " << connStr << std::endl;

    PGconn *conn = PQconnectdb(connStr.c_str());

    if (PQstatus(conn) != CONNECTION_OK)
    {
        std::cerr << "PostgreSQL连接失败: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return 1;
    }

    std::cout << "PostgreSQL连接成功" << std::endl;

    // 测试简单查询
    PGresult *res = PQexec(conn, "SELECT version();");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << "查询失败: " << PQresultErrorMessage(res) << std::endl;
        PQclear(res);
        PQfinish(conn);
        return 1;
    }

    std::cout << "PostgreSQL版本: " << PQgetvalue(res, 0, 0) << std::endl;
    PQclear(res);

    // 检查students表是否存在
    res = PQexec(conn, "SELECT EXISTS (SELECT FROM information_schema.tables WHERE table_schema = 'public' AND table_name = 'students');");
    if (PQresultStatus(res) == PGRES_TUPLES_OK)
    {
        std::string exists = PQgetvalue(res, 0, 0);
        std::cout << "students表是否存在: " << exists << std::endl;
    }
    PQclear(res);

    PQfinish(conn);
    std::cout << "=== PostgreSQL 连接测试完成 ===" << std::endl;

    return 0;
}
