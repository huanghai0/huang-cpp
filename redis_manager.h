#ifndef REDIS_MANAGER_H
#define REDIS_MANAGER_H

#include <string>
#include <memory>
#include <hiredis/hiredis.h>
#include "logger.h"

class RedisManager
{
private:
    std::string host;
    int port;
    redisContext *context;
    bool connected;

    // 重连机制
    bool reconnect();

public:
    RedisManager(const std::string &host = "localhost", int port = 6379);
    ~RedisManager();

    // 连接管理
    bool connect();
    void disconnect();
    bool isConnected() const { return connected; }

    // 基本操作
    bool set(const std::string &key, const std::string &value, int expireSeconds = 0);
    std::string get(const std::string &key);
    bool del(const std::string &key);
    bool exists(const std::string &key);
    bool expire(const std::string &key, int seconds);

    // 哈希表操作
    bool hset(const std::string &key, const std::string &field, const std::string &value);
    std::string hget(const std::string &key, const std::string &field);
    bool hdel(const std::string &key, const std::string &field);

    // 列表操作
    bool lpush(const std::string &key, const std::string &value);
    bool rpush(const std::string &key, const std::string &value);
    std::vector<std::string> lrange(const std::string &key, int start = 0, int end = -1);

    // 集合操作
    bool sadd(const std::string &key, const std::string &member);
    bool srem(const std::string &key, const std::string &member);
    std::vector<std::string> smembers(const std::string &key);

    // 测试连接
    bool ping();
};

#endif // REDIS_MANAGER_H
