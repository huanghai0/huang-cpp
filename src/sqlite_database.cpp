#include "sqlite_database.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include "logger.h"

SQLiteDatabase::SQLiteDatabase(const ConfigManager &configManager)
    : db(nullptr), dbPath(configManager.getSqliteDatabasePath())
{
}

SQLiteDatabase::SQLiteDatabase(const std::string &path)
    : db(nullptr), dbPath(path)
{
}

SQLiteDatabase::~SQLiteDatabase()
{
    close();
}

bool SQLiteDatabase::open()
{
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK)
    {
        Logger::error("无法打开SQLite数据库: {}", sqlite3_errmsg(db));
        return false;
    }

    // 创建学生表
    if (!createStudentTable())
    {
        Logger::error("创建学生表失败");
        return false;
    }

    Logger::info("SQLite数据库连接成功: {}", dbPath);
    return true;
}

void SQLiteDatabase::close()
{
    if (db)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool SQLiteDatabase::createStudentTable()
{
    const char *sql = "CREATE TABLE IF NOT EXISTS students ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "name TEXT NOT NULL,"
                      "age INTEGER NOT NULL,"
                      "className TEXT NOT NULL);";

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        Logger::error("SQL错误: {}", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

int SQLiteDatabase::addStudent(const Student &student)
{
    const char *sql = "INSERT INTO students (name, age, className) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, student.getName().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, student.getAge());
    sqlite3_bind_text(stmt, 3, student.getClassName().c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        Logger::error("执行SQL语句失败: {}", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    int studentId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);

    Logger::info("添加学生成功，ID: {}", studentId);
    return studentId;
}

bool SQLiteDatabase::updateStudent(int id, const Student &student)
{
    const char *sql = "UPDATE students SET name = ?, age = ?, className = ? WHERE id = ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, student.getName().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, student.getAge());
    sqlite3_bind_text(stmt, 3, student.getClassName().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        Logger::error("执行SQL语句失败: {}", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    bool success = sqlite3_changes(db) > 0;
    sqlite3_finalize(stmt);

    if (success)
    {
        Logger::info("更新学生成功，ID: {}", id);
    }

    return success;
}

bool SQLiteDatabase::deleteStudent(int id)
{
    const char *sql = "DELETE FROM students WHERE id = ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        Logger::error("执行SQL语句失败: {}", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    bool success = sqlite3_changes(db) > 0;
    sqlite3_finalize(stmt);

    if (success)
    {
        Logger::info("删除学生成功，ID: {}", id);
    }

    return success;
}

Student SQLiteDatabase::getStudent(int id)
{
    const char *sql = "SELECT name, age, className FROM students WHERE id = ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return Student();
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        int age = sqlite3_column_int(stmt, 1);
        std::string className = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        sqlite3_finalize(stmt);

        Logger::info("从数据库获取学生，ID: {}", id);
        return Student(name, age, className);
    }

    sqlite3_finalize(stmt);
    return Student();
}

std::vector<std::pair<int, Student>> SQLiteDatabase::getAllStudents()
{
    std::vector<std::pair<int, Student>> students;
    const char *sql = "SELECT id, name, age, className FROM students;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return students;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        int age = sqlite3_column_int(stmt, 2);
        std::string className = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        students.emplace_back(id, Student(name, age, className));
    }

    sqlite3_finalize(stmt);
    Logger::info("从数据库获取所有学生，数量: {}", students.size());
    return students;
}

int SQLiteDatabase::getStudentCount()
{
    const char *sql = "SELECT COUNT(*) FROM students;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        Logger::error("准备SQL语句失败: {}", sqlite3_errmsg(db));
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }

    sqlite3_finalize(stmt);
    return -1;
}
