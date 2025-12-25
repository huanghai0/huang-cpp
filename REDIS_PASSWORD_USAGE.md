# Redis密码功能使用说明

## 概述

本项目已成功添加对带密码的Redis连接的支持。现在可以在配置文件中设置Redis密码，系统将自动使用密码进行认证。

## 修改内容

### 1. RedisManager类
- 添加了`password`成员变量
- 修改了构造函数以接受密码参数
- 在`connect()`方法中添加了AUTH命令认证逻辑
- 当密码不为空时，在连接后执行`AUTH <password>`命令

### 2. ConfigManager类
- 添加了`getRedisPassword()`方法
- 从配置文件的`redis.password`字段读取密码
- 如果密码字段不存在或为空，返回空字符串

### 3. DatabaseManager类
- 修改了构造函数，将Redis密码传递给RedisManager
- 确保Redis连接使用正确的认证信息

### 4. 配置文件
- 在`config.json`和`config_postgresql.json`中添加了`redis.password`字段
- 默认值为空字符串，表示不使用密码
- 如果需要密码认证，可以设置为实际的密码值

## 使用方法

### 1. 配置Redis密码

在配置文件的`redis`部分添加`password`字段：

```json
{
    "redis": {
        "host": "110.42.203.226",
        "port": 6379,
        "password": "your_redis_password_here",
        "timeout_seconds": 1.5
    }
}
```

### 2. 代码中使用

#### 直接使用RedisManager：

```cpp
#include "redis_manager.h"

// 创建带密码的RedisManager实例
RedisManager redisManager("localhost", 6379, "your_password");

// 连接Redis（会自动进行密码认证）
if (redisManager.connect()) {
    // 连接成功，可以执行Redis操作
    redisManager.set("key", "value");
    std::string value = redisManager.get("key");
}
```

#### 通过ConfigManager使用：

```cpp
#include "config_manager.h"
#include "redis_manager.h"

ConfigManager configManager;
if (configManager.isLoaded()) {
    // 使用配置中的Redis连接信息（包括密码）
    RedisManager redisManager(
        configManager.getRedisHost(),
        configManager.getRedisPort(),
        configManager.getRedisPassword()
    );
    
    if (redisManager.connect()) {
        // 连接成功
    }
}
```

#### 通过DatabaseManager使用（推荐）：

```cpp
#include "database_manager.h"

ConfigManager configManager;
DatabaseManager dbManager(configManager);

if (dbManager.open()) {
    // DatabaseManager会自动处理Redis连接和密码认证
    // 可以使用缓存功能
    Student student = dbManager.getStudent(1); // 会自动使用Redis缓存
}
```

## 测试

已创建测试程序`test_redis_password.cpp`来验证密码功能：

```bash
# 编译测试程序
g++ -std=c++17 -I./include -I./include/huangh-cpp -I./third_party \
    test_redis_password.cpp \
    src/config_manager.cpp \
    src/redis_manager.cpp \
    src/logger.cpp \
    -o test_redis_password \
    -lhiredis -lfmt -lspdlog -lnlohmann_json

# 运行测试
./test_redis_password
```

## 注意事项

1. **向后兼容性**：如果配置文件中没有`password`字段，系统会使用空字符串作为默认值，保持向后兼容。

2. **空密码处理**：当密码为空字符串时，不会执行AUTH命令，直接连接Redis。

3. **错误处理**：如果密码错误，Redis连接会失败，并记录错误日志。

4. **安全性**：密码以明文形式存储在配置文件中，请确保配置文件的安全访问权限。

## 验证修改

要验证修改是否成功，可以检查以下文件：

1. `include/huangh-cpp/redis_manager.h` - 添加了password参数
2. `src/redis_manager.cpp` - 添加了AUTH命令处理
3. `include/huangh-cpp/config_manager.h` - 添加了getRedisPassword()方法
4. `src/config_manager.cpp` - 实现了getRedisPassword()方法
5. `src/database_manager.cpp` - 传递密码参数给RedisManager
6. `config.json`和`config_postgresql.json` - 添加了password字段

## 总结

通过以上修改，项目现在完全支持带密码的Redis连接。用户只需在配置文件中设置`redis.password`字段，系统就会自动使用密码进行Redis认证。
