#include <iostream>
#include <libpq-fe.h>

// g++ -o test_postgresql_error test_postgresql_error.cpp -lpq
int main()
{
    std::cout << "测试 PostgreSQL 连接错误..." << std::endl;

    // 使用错误的密码
    std::string connStr = "host=192.168.2.146 port=5432 dbname=demo1 user=postgres password=deju@2025 connect_timeout=5";

    PGconn *conn = PQconnectdb(connStr.c_str());

    if (PQstatus(conn) != CONNECTION_OK)
    {
        std::cerr << "PostgreSQL连接失败: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return 1;
    }

    std::cout << "连接成功" << std::endl;
    PQfinish(conn);
    return 0;
}
