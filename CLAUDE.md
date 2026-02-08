# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Build the project
cd build && make

# Run the program
./bin/huangh-cpp
```

The program supports two modes via command-line argument:
- `./bin/huangh-cpp 1` - Run STL collection examples
- `./bin/huangh-cpp 2` - Start HTTP server (default if no argument provided, will prompt)

## Testing the HTTP Server

The HTTP server runs on `http://localhost:8080` by default (configurable via [config.json](config.json)). Test with curl:

```bash
# Add a student
curl -X POST http://localhost:8080/students \
  -H "Content-Type: application/json" \
  -d '{"name":"张三","age":20,"className":"计算机科学1班"}'

# Get all students
curl http://localhost:8080/students

# Get specific student
curl http://localhost:8080/students/1

# Update student
curl -X PUT http://localhost:8080/students/1 \
  -H "Content-Type: application/json" \
  -d '{"name":"张三丰","age":21,"className":"计算机科学1班"}'

# Delete student
curl -X DELETE http://localhost:8080/students/1

# Health check
curl http://localhost:8080/health
```

## Architecture Overview

### Database Abstraction Layer

The project uses a **Strategy Pattern** for database operations:

- **Interface**: [database_interface.h](include/huangh-cpp/database_interface.h) - Defines contract for all database implementations
- **Implementations**:
  - [sqlite_database.h/cpp](include/huangh-cpp/sqlite_database.h) - SQLite3 (file-based, default)
  - [postgresql_database.h/cpp](include/huangh-cpp/postgresql_database.h) - PostgreSQL with connection pooling
- **Manager**: [database_manager.h/cpp](include/huangh-cpp/database_manager.h) - Factory pattern + caching layer

Database type is selected via [config.json](config.json) under `database.type` (either "sqlite" or "postgresql").

### Caching Layer

[database_manager.cpp](src/database_manager.cpp) implements a Redis cache layer with configurable expiration times. The caching strategy:
- Cache keys are prefixed by type (`student:`, `students:`, `count:`)
- Writes invalidate relevant cache entries
- Reads check cache first, then database
- Graceful degradation if Redis is unavailable

### HTTP Server

[http_server.cpp](src/http_server.cpp) uses cpp-httplib (single-header in [third_party/httplib.h](third_party/httplib.h)) for RESTful API endpoints. All endpoints return JSON using nlohmann/json.

### Configuration

[config_manager.h/cpp](include/huangh-cpp/config_manager.h) loads [config.json](config.json) with these sections:
- `database` - Database type and connection settings
- `redis` - Redis host, port, password, timeout
- `server` - HTTP server host and port
- `cache` - Expiration times for different cache types

### Logging

[logger.h/cpp](include/huangh-cpp/logger.h) provides a singleton logger using spdlog with both file and console sinks. Initialize before use: `Logger::initialize("server.log")`

## Dependencies

- CMake 3.10+, C++17
- spdlog (logging)
- fmt (formatting)
- nlohmann/json (JSON handling)
- sqlite3 (SQLite database)
- PostgreSQL (libpq)
- hiredis (Redis client)
- cpp-httplib (single-header HTTP server in [third_party/](third_party/))

## Key Files for Understanding

- [src/main.cpp](src/main.cpp) - Entry point, mode selection
- [src/http_server.cpp](src/http_server.cpp) - REST API endpoints
- [src/database_manager.cpp](src/database_manager.cpp) - Database operations + caching logic
- [src/sqlite_database.cpp](src/sqlite_database.cpp) vs [src/postgresql_database.cpp](src/postgresql_database.cpp) - Database implementations
- [config.json](config.json) - Application configuration
