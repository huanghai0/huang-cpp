#include "redis_manager.h"
#include <vector>
#include <sstream>

RedisManager::RedisManager(const std::string &host, int port, const std::string &password)
    : host(host), port(port), password(password), context(nullptr), connected(false)
{
}

RedisManager::~RedisManager()
{
    disconnect();
}

bool RedisManager::connect()
{
    if (connected)
    {
        return true;
    }

    struct timeval timeout = {1, 500000}; // 1.5 seconds
    context = redisConnectWithTimeout(host.c_str(), port, timeout);

    if (context == nullptr || context->err)
    {
        if (context)
        {
            Logger::error("Redis连接错误: {}", context->errstr);
            redisFree(context);
            context = nullptr;
        }
        else
        {
            Logger::error("无法分配Redis连接");
        }
        connected = false;
        return false;
    }

    // 如果设置了密码，进行认证
    if (!password.empty())
    {
        redisReply *reply = (redisReply *)redisCommand(context, "AUTH %s", password.c_str());
        if (reply == nullptr)
        {
            Logger::error("Redis AUTH命令失败: {}", context->errstr);
            redisFree(context);
            context = nullptr;
            connected = false;
            return false;
        }

        bool authSuccess = (reply->type != REDIS_REPLY_ERROR);
        freeReplyObject(reply);

        if (!authSuccess)
        {
            Logger::error("Redis认证失败: 密码错误");
            redisFree(context);
            context = nullptr;
            connected = false;
            return false;
        }

        Logger::info("Redis认证成功");
    }

    connected = true;
    Logger::info("Redis连接成功: {}:{}", host, port);
    return true;
}

void RedisManager::disconnect()
{
    if (context)
    {
        redisFree(context);
        context = nullptr;
    }
    connected = false;
}

bool RedisManager::reconnect()
{
    disconnect();
    return connect();
}

bool RedisManager::set(const std::string &key, const std::string &value, int expireSeconds)
{
    if (!connected && !connect())
    {
        return false;
    }

    redisReply *reply = nullptr;
    if (expireSeconds > 0)
    {
        reply = (redisReply *)redisCommand(context, "SETEX %s %d %s",
                                           key.c_str(), expireSeconds, value.c_str());
    }
    else
    {
        reply = (redisReply *)redisCommand(context, "SET %s %s",
                                           key.c_str(), value.c_str());
    }

    if (reply == nullptr)
    {
        Logger::error("Redis SET命令失败: {}", context->errstr);
        reconnect();
        return false;
    }

    bool success = (reply->type != REDIS_REPLY_ERROR);
    if (!success)
    {
        Logger::error("Redis SET命令错误: {}", reply->str);
    }

    freeReplyObject(reply);
    return success;
}

std::string RedisManager::get(const std::string &key)
{
    if (!connected && !connect())
    {
        return "";
    }

    redisReply *reply = (redisReply *)redisCommand(context, "GET %s", key.c_str());
    if (reply == nullptr)
    {
        Logger::error("Redis GET命令失败: {}", context->errstr);
        reconnect();
        return "";
    }

    std::string result;
    if (reply->type == REDIS_REPLY_STRING)
    {
        result = std::string(reply->str, reply->len);
    }
    else if (reply->type == REDIS_REPLY_NIL)
    {
        // 键不存在，返回空字符串
    }
    else
    {
        Logger::error("Redis GET命令错误: 类型 {}", reply->type);
    }

    freeReplyObject(reply);
    return result;
}

bool RedisManager::del(const std::string &key)
{
    if (!connected && !connect())
    {
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "DEL %s", key.c_str());
    if (reply == nullptr)
    {
        Logger::error("Redis DEL命令失败: {}", context->errstr);
        reconnect();
        return false;
    }

    bool success = (reply->type != REDIS_REPLY_ERROR);
    if (!success)
    {
        Logger::error("Redis DEL命令错误: {}", reply->str);
    }

    freeReplyObject(reply);
    return success;
}

bool RedisManager::exists(const std::string &key)
{
    if (!connected && !connect())
    {
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "EXISTS %s", key.c_str());
    if (reply == nullptr)
    {
        Logger::error("Redis EXISTS命令失败: {}", context->errstr);
        reconnect();
        return false;
    }

    bool exists = false;
    if (reply->type == REDIS_REPLY_INTEGER)
    {
        exists = (reply->integer == 1);
    }
    else
    {
        Logger::error("Redis EXISTS命令错误: 类型 {}", reply->type);
    }

    freeReplyObject(reply);
    return exists;
}

bool RedisManager::expire(const std::string &key, int seconds)
{
    if (!connected && !connect())
    {
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "EXPIRE %s %d", key.c_str(), seconds);
    if (reply == nullptr)
    {
        Logger::error("Redis EXPIRE命令失败: {}", context->errstr);
        reconnect();
        return false;
    }

    bool success = (reply->type != REDIS_REPLY_ERROR);
    if (!success)
    {
        Logger::error("Redis EXPIRE命令错误: {}", reply->str);
    }

    freeReplyObject(reply);
    return success;
}

bool RedisManager::hset(const std::string &key, const std::string &field, const std::string &value)
{
    if (!connected && !connect())
    {
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "HSET %s %s %s",
                                                   key.c_str(), field.c_str(), value.c_str());
    if (reply == nullptr)
    {
        Logger::error("Redis HSET命令失败: {}", context->errstr);
        reconnect();
        return false;
    }

    bool success = (reply->type != REDIS_REPLY_ERROR);
    if (!success)
    {
        Logger::error("Redis HSET命令错误: {}", reply->str);
    }

    freeReplyObject(reply);
    return success;
}

std::string RedisManager::hget(const std::string &key, const std::string &field)
{
    if (!connected && !connect())
    {
        return "";
    }

    redisReply *reply = (redisReply *)redisCommand(context, "HGET %s %s", key.c_str(), field.c_str());
    if (reply == nullptr)
    {
        Logger::error("Redis HGET命令失败: {}", context->errstr);
        reconnect();
        return "";
    }

    std::string result;
    if (reply->type == REDIS_REPLY_STRING)
    {
        result = std::string(reply->str, reply->len);
    }
    else if (reply->type == REDIS_REPLY_NIL)
    {
        // 字段不存在，返回空字符串
    }
    else
    {
        Logger::error("Redis HGET命令错误: 类型 {}", reply->type);
    }

    freeReplyObject(reply);
    return result;
}

bool RedisManager::hdel(const std::string &key, const std::string &field)
{
    if (!connected && !connect())
    {
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "HDEL %s %s", key.c_str(), field.c_str());
    if (reply == nullptr)
    {
        Logger::error("Redis HDEL命令失败: {}", context->errstr);
        reconnect();
        return false;
    }

    bool success = (reply->type != REDIS_REPLY_ERROR);
    if (!success)
    {
        Logger::error("Redis HDEL命令错误: {}", reply->str);
    }

    freeReplyObject(reply);
    return success;
}

bool RedisManager::lpush(const std::string &key, const std::string &value)
{
    if (!connected && !connect())
    {
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "LPUSH %s %s", key.c_str(), value.c_str());
    if (reply == nullptr)
    {
        Logger::error("Redis LPUSH命令失败: {}", context->errstr);
        reconnect();
        return false;
    }

    bool success = (reply->type != REDIS_REPLY_ERROR);
    if (!success)
    {
        Logger::error("Redis LPUSH命令错误: {}", reply->str);
    }

    freeReplyObject(reply);
    return success;
}

bool RedisManager::rpush(const std::string &key, const std::string &value)
{
    if (!connected && !connect())
    {
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "RPUSH %s %s", key.c_str(), value.c_str());
    if (reply == nullptr)
    {
        Logger::error("Redis RPUSH命令失败: {}", context->errstr);
        reconnect();
        return false;
    }

    bool success = (reply->type != REDIS_REPLY_ERROR);
    if (!success)
    {
        Logger::error("Redis RPUSH命令错误: {}", reply->str);
    }

    freeReplyObject(reply);
    return success;
}

std::vector<std::string> RedisManager::lrange(const std::string &key, int start, int end)
{
    std::vector<std::string> result;

    if (!connected && !connect())
    {
        return result;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "LRANGE %s %d %d", key.c_str(), start, end);
    if (reply == nullptr)
    {
        Logger::error("Redis LRANGE命令失败: {}", context->errstr);
        reconnect();
        return result;
    }

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; i++)
        {
            redisReply *element = reply->element[i];
            if (element->type == REDIS_REPLY_STRING)
            {
                result.push_back(std::string(element->str, element->len));
            }
        }
    }
    else if (reply->type != REDIS_REPLY_ERROR)
    {
        Logger::error("Redis LRANGE命令错误: 类型 {}", reply->type);
    }

    freeReplyObject(reply);
    return result;
}

bool RedisManager::sadd(const std::string &key, const std::string &member)
{
    if (!connected && !connect())
    {
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "SADD %s %s", key.c_str(), member.c_str());
    if (reply == nullptr)
    {
        Logger::error("Redis SADD命令失败: {}", context->errstr);
        reconnect();
        return false;
    }

    bool success = (reply->type != REDIS_REPLY_ERROR);
    if (!success)
    {
        Logger::error("Redis SADD命令错误: {}", reply->str);
    }

    freeReplyObject(reply);
    return success;
}

bool RedisManager::srem(const std::string &key, const std::string &member)
{
    if (!connected && !connect())
    {
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "SREM %s %s", key.c_str(), member.c_str());
    if (reply == nullptr)
    {
        Logger::error("Redis SREM命令失败: {}", context->errstr);
        reconnect();
        return false;
    }

    bool success = (reply->type != REDIS_REPLY_ERROR);
    if (!success)
    {
        Logger::error("Redis SREM命令错误: {}", reply->str);
    }

    freeReplyObject(reply);
    return success;
}

std::vector<std::string> RedisManager::smembers(const std::string &key)
{
    std::vector<std::string> result;

    if (!connected && !connect())
    {
        return result;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS %s", key.c_str());
    if (reply == nullptr)
    {
        Logger::error("Redis SMEMBERS命令失败: {}", context->errstr);
        reconnect();
        return result;
    }

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; i++)
        {
            redisReply *element = reply->element[i];
            if (element->type == REDIS_REPLY_STRING)
            {
                result.push_back(std::string(element->str, element->len));
            }
        }
    }
    else if (reply->type != REDIS_REPLY_ERROR)
    {
        Logger::error("Redis SMEMBERS命令错误: 类型 {}", reply->type);
    }

    freeReplyObject(reply);
    return result;
}

bool RedisManager::ping()
{
    if (!connected && !connect())
    {
        return false;
    }

    redisReply *reply = (redisReply *)redisCommand(context, "PING");
    if (reply == nullptr)
    {
        Logger::error("Redis PING命令失败: {}", context->errstr);
        reconnect();
        return false;
    }

    bool success = (reply->type == REDIS_REPLY_STATUS &&
                    std::string(reply->str, reply->len) == "PONG");

    freeReplyObject(reply);
    return success;
}
