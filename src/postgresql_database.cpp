#include "postgresql_database.h"
#include <libpq-fe.h>
#include <iostream>
#include <sstream>
#include "logger.h"

PostgreSQLDatabase::PostgreSQLDatabase(const ConfigManager &configManager)
    : configManager(&configManager)
{
    connectionPool = std::make_shared<ConnectionPool>();
}

PostgreSQLDatabase::~PostgreSQLDatabase()
{
    close();
}

bool PostgreSQLDatabase::createConnectionPool()
{
    if (!configManager)
    {
        Logger::error("配置管理器未初始化");
        return false;
    }

    // 构建连接字符串
    std::stringstream connStr;
    connStr << "host=" << configManager->getPostgresqlHost()
            << " port=" << configManager->getPostgresqlPort()
            << " dbname=" << configManager->getPostgresqlDatabase()
            << " user=" << configManager->getPostgresqlUsername()
            << " password=" << configManager->getPostgresqlPassword()
            << " connect_timeout=" << configManager->getPostgresqlConnectionTimeout();

    connectionPool->connectionString = connStr.str();
    connectionPool->maxSize = configManager->getPostgresqlConnectionPoolSize();

    // 创建初始连接
    for (size_t i = 0; i < connectionPool->maxSize; ++i)
    {
        PGconn *conn = PQconnectdb(connectionPool->connectionString.c_str());
        if (PQstatus(conn) != CONNECTION_OK)
        {
            Logger::error("PostgreSQL连接失败: {}", PQerrorMessage(conn));
            PQfinish(conn);
            return false;
        }
        connectionPool->connections.push_back(conn);
    }

    Logger::info("PostgreSQL连接池创建成功，大小: {}", connectionPool->maxSize);
    return true;
}

PGconn *PostgreSQLDatabase::acquireConnection()
{
    std::lock_guard<std::mutex> lock(connectionPool->mutex);

    if (connectionPool->connections.empty())
    {
        // 连接池为空，创建新连接
        PGconn *conn = PQconnectdb(connectionPool->connectionString.c_str());
        if (PQstatus(conn) != CONNECTION_OK)
        {
            Logger::error("无法创建新连接: {}", PQerrorMessage(conn));
            PQfinish(conn);
            return nullptr;
        }
        return conn;
    }

    PGconn *conn = connectionPool->connections.back();
    connectionPool->connections.pop_back();
    return conn;
}

void PostgreSQLDatabase::releaseConnection(PGconn *conn)
{
    if (!conn)
        return;

    std::lock_guard<std::mutex> lock(connectionPool->mutex);

    if (connectionPool->connections.size() < connectionPool->maxSize)
    {
        connectionPool->connections.push_back(conn);
    }
    else
    {
        PQfinish(conn);
    }
}

PGresult *PostgreSQLDatabase::executeQuery(PGconn *conn, const std::string &sql, const std::vector<std::string> &params)
{
    if (!conn)
    {
        Logger::error("数据库连接无效");
        return nullptr;
    }

    // 准备参数
    const char **paramValues = nullptr;
    if (!params.empty())
    {
        paramValues = new const char *[params.size()];
        for (size_t i = 0; i < params.size(); ++i)
        {
            paramValues[i] = params[i].c_str();
        }
    }

    PGresult *result = PQexecParams(conn,
                                    sql.c_str(),
                                    params.size(),
                                    nullptr, // 参数类型（使用默认）
                                    paramValues,
                                    nullptr, // 参数长度（文本格式）
                                    nullptr, // 参数格式（文本）
                                    0);      // 结果格式（文本）

    if (paramValues)
    {
        delete[] paramValues;
    }

    if (PQresultStatus(result) != PGRES_COMMAND_OK && PQresultStatus(result) != PGRES_TUPLES_OK)
    {
        Logger::error("SQL执行错误: {}", PQresultErrorMessage(result));
        PQclear(result);
        return nullptr;
    }

    return result;
}

bool PostgreSQLDatabase::open()
{
    if (!createConnectionPool())
    {
        Logger::error("无法创建PostgreSQL连接池");
        return false;
    }

    // 创建表
    if (!createStudentTable())
    {
        Logger::error("创建学生表失败");
        return false;
    }

    Logger::info("PostgreSQL数据库连接成功");
    return true;
}

void PostgreSQLDatabase::close()
{
    if (!connectionPool)
        return;

    std::lock_guard<std::mutex> lock(connectionPool->mutex);
    for (PGconn *conn : connectionPool->connections)
    {
        PQfinish(conn);
    }
    connectionPool->connections.clear();
    Logger::info("PostgreSQL数据库连接已关闭");
}

bool PostgreSQLDatabase::createStudentTable()
{
    PGconn *conn = acquireConnection();
    if (!conn)
        return false;

    const char *sql = "CREATE TABLE IF NOT EXISTS students ("
                      "id SERIAL PRIMARY KEY,"
                      "name VARCHAR(255) NOT NULL,"
                      "age INTEGER NOT NULL,"
                      "className VARCHAR(255) NOT NULL);";

    PGresult *result = executeQuery(conn, sql);
    releaseConnection(conn);

    if (!result)
    {
        return false;
    }

    PQclear(result);
    return true;
}

int PostgreSQLDatabase::addStudent(const Student &student)
{
    PGconn *conn = acquireConnection();
    if (!conn)
        return -1;

    std::string sql = "INSERT INTO students (name, age, className) VALUES ($1, $2, $3) RETURNING id;";
    std::vector<std::string> params = {student.getName(), std::to_string(student.getAge()), student.getClassName()};

    PGresult *result = executeQuery(conn, sql, params);
    releaseConnection(conn);

    if (!result)
    {
        return -1;
    }

    int studentId = std::stoi(PQgetvalue(result, 0, 0));
    PQclear(result);

    Logger::info("添加学生成功，ID: {}", studentId);
    return studentId;
}

bool PostgreSQLDatabase::updateStudent(int id, const Student &student)
{
    PGconn *conn = acquireConnection();
    if (!conn)
        return false;

    std::string sql = "UPDATE students SET name = $1, age = $2, className = $3 WHERE id = $4;";
    std::vector<std::string> params = {student.getName(), std::to_string(student.getAge()), student.getClassName(), std::to_string(id)};

    PGresult *result = executeQuery(conn, sql, params);
    releaseConnection(conn);

    if (!result)
    {
        return false;
    }

    bool success = PQcmdTuples(result)[0] != '0';
    PQclear(result);

    if (success)
    {
        Logger::info("更新学生成功，ID: {}", id);
    }

    return success;
}

bool PostgreSQLDatabase::deleteStudent(int id)
{
    PGconn *conn = acquireConnection();
    if (!conn)
        return false;

    std::string sql = "DELETE FROM students WHERE id = $1;";
    std::vector<std::string> params = {std::to_string(id)};

    PGresult *result = executeQuery(conn, sql, params);
    releaseConnection(conn);

    if (!result)
    {
        return false;
    }

    bool success = PQcmdTuples(result)[0] != '0';
    PQclear(result);

    if (success)
    {
        Logger::info("删除学生成功，ID: {}", id);
    }

    return success;
}

Student PostgreSQLDatabase::getStudent(int id)
{
    PGconn *conn = acquireConnection();
    if (!conn)
        return Student();

    std::string sql = "SELECT name, age, className FROM students WHERE id = $1;";
    std::vector<std::string> params = {std::to_string(id)};

    PGresult *result = executeQuery(conn, sql, params);
    releaseConnection(conn);

    if (!result)
    {
        return Student();
    }

    if (PQntuples(result) == 0)
    {
        PQclear(result);
        return Student();
    }

    std::string name = PQgetvalue(result, 0, 0);
    int age = std::stoi(PQgetvalue(result, 0, 1));
    std::string className = PQgetvalue(result, 0, 2);

    PQclear(result);

    Logger::info("从数据库获取学生，ID: {}", id);
    return Student(name, age, className);
}

std::vector<std::pair<int, Student>> PostgreSQLDatabase::getAllStudents()
{
    std::vector<std::pair<int, Student>> students;

    PGconn *conn = acquireConnection();
    if (!conn)
        return students;

    std::string sql = "SELECT id, name, age, className FROM students;";
    PGresult *result = executeQuery(conn, sql);
    releaseConnection(conn);

    if (!result)
    {
        return students;
    }

    int numRows = PQntuples(result);
    for (int i = 0; i < numRows; ++i)
    {
        int id = std::stoi(PQgetvalue(result, i, 0));
        std::string name = PQgetvalue(result, i, 1);
        int age = std::stoi(PQgetvalue(result, i, 2));
        std::string className = PQgetvalue(result, i, 3);
        students.emplace_back(id, Student(name, age, className));
    }

    PQclear(result);
    Logger::info("从数据库获取所有学生，数量: {}", students.size());
    return students;
}

int PostgreSQLDatabase::getStudentCount()
{
    PGconn *conn = acquireConnection();
    if (!conn)
        return -1;

    std::string sql = "SELECT COUNT(*) FROM students;";
    PGresult *result = executeQuery(conn, sql);
    releaseConnection(conn);

    if (!result)
    {
        return -1;
    }

    int count = std::stoi(PQgetvalue(result, 0, 0));
    PQclear(result);

    return count;
}
