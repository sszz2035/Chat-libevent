# 📡 Chat Server - 高性能聊天服务器

基于 **libevent** 和 **C++17** 开发的高性能、事件驱动的聊天服务器，支持多线程并发、即时通讯、群组聊天、文件传输等功能。

---

## 📋 目录

- [项目概述](#🎯 项目概述)
- [功能特性](#✨ 功能特性)
- [技术架构](#🏗️ 技术架构)
- [项目结构](# 📁 项目结构)
- [核心组件](#🔧 核心组件)
- [线程模型](#🧵 线程模型)
- [数据库设计](#💾 数据库设计)
- [构建和运行](#🛠️ 构建和运行)
- [配置说明](#⚙️ 配置说明)
- [API 接口](#🔌 API 接口)
- [日志系统](#📝 日志系统)
- [故障排除](#🔧 故障排除)

---

## 🎯 项目概述

Chat Server 是聊天系统的服务端核心，采用 **Reactor 模式 + 线程池** 的架构设计，能够高效处理大量并发连接。服务器基于 libevent 事件库实现异步 I/O，使用多线程提高并发处理能力。

### 设计理念

- **高性能**: 基于 libevent 的事件驱动 I/O
- **可扩展**: 线程池模型支持水平扩展
- **稳定性**: 完善的错误处理和资源管理
- **可维护**: 清晰的模块划分和代码结构

---

## ✨ 功能特性

### 🔐 用户管理
- ✅ 用户注册与登录验证
- ✅ 密码安全校验
- ✅ 用户信息存储（用户名、好友列表、群组列表）
- ✅ 在线状态管理

### 💬 消息系统
- ✅ 私聊消息实时转发
- ✅ 群聊消息广播
- ✅ 文本消息处理
- ✅ 图片传输（Base64 编码）
- ✅ 文件传输（大文件分块传输）
- ✅ 离线消息状态反馈

### 👥 群组管理
- ✅ 群组创建与成员管理
- ✅ 群信息缓存（内存）
- ✅ 群成员在线状态追踪
- ✅ 群聊消息分发

### 🏗️ 系统功能
- ✅ 线程池管理（默认 3 线程）
- ✅ 数据库连接管理
- ✅ Snowflake ID 生成
- ✅ 日志记录系统
- ✅ 优雅的资源释放

---

## 🏗️ 技术架构

```
┌─────────────────────────────────────────┐
│           Chat Server                   │
├─────────────────────────────────────────┤
│  📡 Network I/O Layer (libevent)        │
│  ┌─────────────────────────────────────┐│
│  │  Event Base (主线程)                 ││
│  │  - 监听连接                          ││
│  │  - 轮询分配                          ││
│  └─────────────────────────────────────┘│
│  ┌─────────────────────────────────────┐│
│  │  Event Base (工作线程池)             ││
│  │  - 处理客户端请求                    ││
│  │  - 事件循环                          ││
│  └─────────────────────────────────────┘│
├─────────────────────────────────────────┤
│  🔄 Business Logic Layer                │
│  ┌─────────────────────────────────────┐│
│  │  Thread Functions                   ││
│  │  - 用户认证 (thread_register)        ││
│  │  - 用户登录 (thread_login)           ││
│  │  - 私聊处理 (thread_private_chat)    ││
│  │  - 群聊处理 (thread_group_chat)      ││
│  │  - 图片传输 (thread_image_private)   ││
│  └─────────────────────────────────────┘│
├─────────────────────────────────────────┤
│  💾 Data Access Layer                   │
│  ┌─────────────────────────────────────┐│
│  │  DataBase (MySQL)                   ││
│  │  - 用户信息管理                      ││
│  │  - 群组信息管理                      ││
│  │  - 读写锁保护                       ││
│  └─────────────────────────────────────┘│
│  ┌─────────────────────────────────────┐│
│  │  ChatInfo (内存缓存)                ││
│  │  - 在线用户列表                      ││
│  │  - 群组信息缓存                      ││
│  └─────────────────────────────────────┘│
├─────────────────────────────────────────┤
│  🔧 Utils Layer                         │
│  - EventBaseDeleter (RAII 包装)         │
│  - Log System (日志记录)                │
│  - SnowflakeIDGenerator (ID 生成)       │
│  - JSON Parser (JsonCpp)               │
└─────────────────────────────────────────┘
```

### 核心技术栈

| 组件 | 技术选型 | 版本要求 |
|------|----------|----------|
| 网络库 | libevent | 2.1.0+ |
| 数据库 | MySQL | 5.7+ |
| 序列化 | JsonCpp | 1.9.0+ |
| 编程语言 | C++ | 17+ |
| 构建工具 | CMake | 3.19+ |

---

## 📁 项目结构

```
chatServer/
│
├── header/                          # 头文件目录
│   ├── chat_server.h                # 服务器核心类声明
│   ├── chat_thread.h                # 线程池管理声明
│   ├── chat_info.h                  # 在线用户管理声明
│   ├── data_base.h                  # 数据库操作声明
│   ├── event_utils.h                # 事件工具（RAII 包装）
│   ├── log.h                        # 日志系统宏定义
│   └── snowflake_id_generator.h     # Snowflake ID 生成器声明
│
├── src/                             # 源代码目录
│   ├── main.cpp                     # 服务器入口程序
│   ├── chat_server.cpp              # 服务器核心实现
│   ├── chat_thread.cpp              # 线程池实现
│   ├── chat_info.cpp                # 在线用户管理实现
│   ├── data_base.cpp                # 数据库操作实现
│   └── snowflake_id_generator.cpp   # ID 生成器实现
│
├── CMakeLists.txt                   # CMake 构建配置
├── README.md                        # 项目说明文档
└── LICENSE                          # 开源许可证
```

---

## 🔧 核心组件

### 1. ChatServer (服务器核心)

**文件**: `header/chat_server.h`, `src/chat_server.cpp`

**职责**:
- 服务器生命周期管理
- 初始化数据库和线程池
- 启动监听端口
- 分发客户端连接到线程池

**关键方法**:
```cpp
// 启动服务器
void listen(const char* ip, int port);

// 初始化数据库表
void database_init_table();

// 更新群组信息缓存
void server_update_group_info();
```

### 2. ChatThread (线程池)

**文件**: `header/chat_thread.h`, `src/chat_thread.cpp`

**职责**:
- 管理固定大小的线程池（默认 3 个线程）
- 每个线程独立的事件循环
- 处理客户端的读写事件
- 轮询分配客户端连接

**配置参数**:
```cpp
#define POOL_MAX_THREAD_SZ 3        // 最大线程数
#define MAX_PACKET_SIZE 1024*1024*10 // 最大数据包 10MB
#define FILE_STORAGE_PATH "./file_storage" // 文件存储路径
#define CHUNK_SIZE 32768            // 分块大小 32KB
```

**工作流程**:
```
连接建立 → 轮询分配线程 → 创建 bufferevent → 设置读写回调 → 启用读事件 → 等待数据
```

### 3. DataBase (数据库操作)

**文件**: `header/data_base.h`, `src/data_base.cpp`

**职责**:
- MySQL 数据库连接管理
- 用户信息的增删改查
- 群组信息管理
- 使用读写锁保证线程安全

**设计模式**:
- 使用 `std::shared_mutex` 实现读写锁
- 读操作使用共享锁（并发）
- 写操作使用独占锁（安全）

**核心方法**:
```cpp
// 查询操作（读锁）
std::vector<std::map<std::string, std::string>> exec_query_and_fetch_rows(const std::string& sql);

// 更新操作（写锁）
bool exec_update(const std::string& sql);

// 用户注册
bool insert_user(uint64_t uid, const std::string& username, const std::string& password);

// 用户登录验证
bool verify_user(uint64_t uid, const std::string& password);
```

### 4. ChatInfo (在线用户管理)

**文件**: `header/chat_info.h`, `src/chat_info.cpp`

**职责**:
- 维护在线用户链表
- 管理群组信息缓存
- 提供用户查找、添加、删除功能
- 使用互斥锁保证线程安全

**数据结构**:
```cpp
class ChatInfo {
private:
    std::list<User>* online_user;  // 在线用户链表
    std::map<uint64_t, std::list<std::string>>* group_info; // 群组信息

    std::mutex list_mutex; // 保护 online_user
    std::mutex map_mutex;  // 保护 group_info
};
```

**User 结构**:
```cpp
class User {
public:
    uint64_t uid;                      // 用户 ID
    std::string name;                  // 用户名
    struct bufferevent* bufevent;      // 网络事件
};
```

### 5. SnowflakeIDGenerator (ID 生成器)

**文件**: `header/snowflake_id_generator.h`, `src/snowflake_id_generator.cpp`

**职责**:
- 使用 Snowflake 算法生成唯一 ID
- 保证 ID 的趋势递增
- 支持分布式 ID 生成

**ID 结构**:
```
+--------------------------------------------------------------------------+
|  1位  |    41位时间戳    |  5位数据中心ID  |  5位机器ID  |    12位序列号   |
|  0    |   (毫秒级)       |     (0-31)      |   (0-31)    |   (0-4095)     |
+--------------------------------------------------------------------------+
```

**配置**:
```cpp
static const int64_t EPOCH = 1609459200000L; // 2021-01-01 00:00:00
```

---

## 🧵 线程模型

### 架构图

```
┌─────────────────────────────────────────┐
│              Main Thread                │
│  (监听线程)                              │
│  - 监听客户端连接                         │
│  - 接受新连接                            │
│  - 轮询分配到工作线程                     │
└────────────┬────────────────────────────┘
             │
     ┌───────┴────────┬────────┬────────┐
     │                │        │        │
┌────▼────┐     ┌─────▼───┐  ┌─▼─────┐  │
│Thread 1 │     │Thread 2 │  │Thread3│  │
│(工作线程)│     │(工作线程)│  │(工作线程)│
│ - 事件循环│     │ - 事件循环│  │- 事件循环│
│ - 处理请求│     │ - 处理请求│  │ - 处理请求│
└─────────┘     └─────────┘  └───────┘
```

### 工作流程

1. **主线程**启动监听器，绑定 `listener_cb` 回调
2. 当有新连接时，`listener_cb` 被调用
3. 主线程**轮询**选择工作线程（round-robin）
4. 在选中的工作线程中创建 `bufferevent`
5. 设置读写回调 `thread_read_cb` 和 `thread_write_cb`
6. 启用读事件，等待客户端数据
7. 工作线程的事件循环处理所有客户端请求

### 事件回调处理

```cpp
// 监听回调（主线程）
static void listener_cb(struct evconnlistener* listener,
                       evutil_socket_t fd,
                       struct sockaddr* addr,
                       int socklen,
                       void* arg);

// 读事件回调（工作线程）
static void thread_read_cb(struct bufferevent* bev, void* arg);

// 写事件回调（工作线程）
static void thread_write_cb(struct bufferevent* bev, void* arg);

// 事件错误回调
static void thread_event_cb(struct bufferevent* bev,
                           short events,
                           void* arg);
```

---

## 💾 数据库设计

### 数据表结构

#### 1. 用户表 (chat_user)

```sql
CREATE TABLE chat_user (
    uid BIGINT PRIMARY KEY,           -- 用户ID (Snowflake 生成)
    username VARCHAR(128) NOT NULL,   -- 用户名
    password VARCHAR(128) NOT NULL,   -- 密码 (建议加密存储)
    friendlist VARCHAR(4096),         -- 好友列表 (uid 用 | 分隔)
    grouplist VARCHAR(4096),          -- 群组列表 (gid 用 | 分隔)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

**示例数据**:
```
uid: 10001
username: "张三"
password: "abc123"  -- 生产环境应加密
friendlist: "10002|10003|10004"
grouplist: "20001|20002"
```

#### 2. 群组表 (chat_group)

```sql
CREATE TABLE chat_group (
    gid BIGINT PRIMARY KEY,            -- 群ID (Snowflake 生成)
    groupname VARCHAR(128) NOT NULL,   -- 群名
    groupowner VARCHAR(128) NOT NULL,  -- 群主用户名
    groupmember VARCHAR(4096),         -- 成员列表 (uid 用 | 分隔)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

**示例数据**:
```
gid: 20001
groupname: "技术交流群"
groupowner: "张三"
groupmember: "10001|10002|10003|10004"
```

### 数据库连接配置

在 `src/main.cpp` 中配置：

```cpp
// MySQL 连接参数
const char* DB_HOST = "localhost";
const char* DB_USER = "chat_user";
const char* DB_PASS = "password123";
const char* DB_NAME = "chat_system";
int DB_PORT = 3306;
```

**注意**: 生产环境建议：
- 使用专用的数据库用户，限制权限
- 密码使用配置文件或环境变量，不要硬编码
- 启用 SSL 连接

### 数据库操作示例

```cpp
// 1. 插入用户
bool insert_user(uint64_t uid, const std::string& username, const std::string& password) {
    std::string sql = fmt::format(
        "INSERT INTO chat_user (uid, username, password) VALUES ({}, '{}', '{}')",
        uid, username, password
    );
    return exec_update(sql);
}

// 2. 查询用户好友列表
std::string get_friend_list(uint64_t uid) {
    std::string sql = fmt::format(
        "SELECT friendlist FROM chat_user WHERE uid = {}", uid
    );
    auto results = exec_query_and_fetch_rows(sql);
    if (!results.empty()) {
        return results[0]["friendlist"];
    }
    return "";
}

// 3. 更新群组成员
bool update_group_members(uint64_t gid, const std::string& members) {
    std::string sql = fmt::format(
        "UPDATE chat_group SET groupmember = '{}' WHERE gid = {}",
        members, gid
    );
    return exec_update(sql);
}
```

---

## 🛠️ 构建和运行

### 环境要求

- **操作系统**: Linux (推荐) / Windows
- **C++ 编译器**: GCC 9+ / Clang 10+ / MSVC 2019+
- **CMake**: 3.19 或更高版本
- **MySQL**: 5.7 或更高版本
- **依赖库**:
  - libevent 2.1.0+
  - JsonCpp 1.9.0+
  - MySQL Connector/C++
  - fmt 库（可选，用于格式化字符串）

### 安装依赖

#### Ubuntu/Debian

```bash
# 安装编译工具
sudo apt-get update
sudo apt-get install -y build-essential cmake

# 安装 libevent
sudo apt-get install -y libevent-dev

# 安装 JsonCpp
sudo apt-get install -y libjsoncpp-dev

# 安装 MySQL 开发库
sudo apt-get install -y libmysqlclient-dev

# 安装 fmt（可选）
sudo apt-get install -y libfmt-dev
```

#### CentOS/RHEL

```bash
sudo yum install -y gcc-c++ cmake
sudo yum install -y libevent-devel
sudo yum install -y jsoncpp-devel
sudo yum install -y mysql-devel
sudo yum install -y fmt-devel
```

#### Windows (vcpkg)

```bash
# 安装 vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat

# 安装依赖
vcpkg install libevent:x64-windows
vcpkg install jsoncpp:x64-windows
vcpkg install mysql-connector-cpp:x64-windows
vcpkg install fmt:x64-windows
```

### 编译步骤

```bash
# 1. 克隆项目（如果还未克隆）
git clone <项目地址>
cd Chat-libevent/chatServer

# 2. 创建构建目录
mkdir build
cd build

# 3. 配置 CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. 编译
cmake --build .

# 5. 运行服务器
./chatServer
```

### CMake 配置说明

`CMakeLists.txt` 示例：

```cmake
cmake_minimum_required(VERSION 3.19)
project(ChatServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖库
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVENT REQUIRED libevent)

find_package(Threads REQUIRED)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/../header
    ${LIBEVENT_INCLUDE_DIRS}
)

# 源文件
set(SOURCES
    src/main.cpp
    src/chat_server.cpp
    src/chat_thread.cpp
    src/chat_info.cpp
    src/data_base.cpp
    src/snowflake_id_generator.cpp
)

# 创建可执行文件
add_executable(chatServer ${SOURCES})

# 链接库
target_link_libraries(chatServer
    ${LIBEVENT_LIBRARIES}
    mysqlclient
    jsoncpp
    fmt
    Threads::Threads
)

# 设置输出目录
set_target_properties(chatServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
```

---

## ⚙️ 配置说明

### 服务器配置

在 `src/main.cpp` 中修改：

```cpp
// 监听配置
const char* SERVER_IP = "0.0.0.0";  // 监听所有 IP
int SERVER_PORT = 8888;              // 监听端口

// MySQL 配置
const char* DB_HOST = "localhost";
const char* DB_USER = "chat_user";
const char* DB_PASS = "password123";
const char* DB_NAME = "chat_system";
int DB_PORT = 3306;

// 线程池配置 (chat_thread.h)
#define POOL_MAX_THREAD_SZ 3        // 工作线程数
#define MAX_PACKET_SIZE 10485760    // 10MB
#define CHUNK_SIZE 32768            // 32KB
```

### 配置文件（推荐）

生产环境建议使用配置文件而非硬编码：

创建 `config/server.conf`：

```ini
[server]
ip = 0.0.0.0
port = 8888
max_threads = 3
max_packet_size = 10485760
chunk_size = 32768

[database]
host = localhost
user = chat_user
password = password123
database = chat_system
port = 3306

[log]
level = INFO
file = ./logs/server.log
```

然后在代码中加载配置文件。

---

## 🔌 API 接口

### 协议格式

基于 TCP 的自定义协议，使用 **长度前缀 + JSON** 的格式：

```
┌──────────────┬──────────────────┐
│   4字节      │    JSON 数据      │
│  (网络序)     │   (UTF-8 编码)    │
└──────────────┴──────────────────┘
```

**说明**:
- 前 4 字节表示 JSON 数据的长度（网络字节序）
- 后续是完整的 JSON 字符串
- 使用 `htonl()`/`ntohl()` 进行字节序转换

### 消息类型

#### 1. 用户注册

**请求**:
```json
{
    "type": "register",
    "username": "user123",
    "password": "abc123",
    "email": "user@example.com"
}
```

**响应**:
```json
{
    "type": "register_response",
    "status": "success",
    "uid": 10001,
    "message": "注册成功"
}
```

#### 2. 用户登录

**请求**:
```json
{
    "type": "login",
    "username": "user123",
    "password": "abc123"
}
```

**响应**:
```json
{
    "type": "login_response",
    "status": "success",
    "uid": 10001,
    "username": "user123",
    "friendlist": "10002|10003",
    "grouplist": "20001",
    "message": "登录成功"
}
```

#### 3. 私聊消息

**请求**:
```json
{
    "type": "private_chat",
    "from_uid": 10001,
    "to_uid": 10002,
    "content": "你好！",
    "msg_type": "text",
    "timestamp": 1703123456
}
```

**响应**:
```json
{
    "type": "private_chat_response",
    "status": "success",
    "message": "消息已发送"
}
```

**转发给接收者**:
```json
{
    "type": "private_message",
    "from_uid": 10001,
    "from_username": "user123",
    "to_uid": 10002,
    "content": "你好！",
    "msg_type": "text",
    "timestamp": 1703123456
}
```

#### 4. 群聊消息

**请求**:
```json
{
    "type": "group_chat",
    "from_uid": 10001,
    "to_gid": 20001,
    "content": "大家好！",
    "msg_type": "text",
    "timestamp": 1703123456
}
```

**转发给群成员**:
```json
{
    "type": "group_message",
    "from_uid": 10001,
    "from_username": "user123",
    "to_gid": 20001,
    "content": "大家好！",
    "msg_type": "text",
    "timestamp": 1703123456
}
```

#### 5. 图片私聊

**请求**:
```json
{
    "type": "image_private_chat",
    "from_uid": 10001,
    "to_uid": 10002,
    "image_data": "base64_encoded_image_data",
    "image_name": "photo.png",
    "image_size": 1024000
}
```

**响应**:
```json
{
    "type": "image_response",
    "status": "success",
    "file_id": "img_12345",
    "message": "图片发送成功"
}
```

**转发给接收者**:
```json
{
    "type": "image_message",
    "from_uid": 10001,
    "from_username": "user123",
    "to_uid": 10002,
    "image_data": "base64_encoded_image_data",
    "image_name": "photo.png",
    "image_size": 1024000,
    "file_id": "img_12345"
}
```

#### 6. 大文件分块传输

客户端发送第一块：
```json
{
    "type": "image_private_chunk",
    "from_uid": 10001,
    "to_uid": 10002,
    "file_name": "document.pdf",
    "chunk_data": "base64_chunk_data",
    "chunk_index": 0,
    "total_chunks": 10,
    "file_size": 10485760
}
```

服务器响应：
```json
{
    "type": "chunk_response",
    "status": "success",
    "chunk_index": 0,
    "message": "已接收第 0 块"
}
```

全部接收完成后，服务器组装文件并转发。

#### 7. 创建群组

**请求**:
```json
{
    "type": "create_group",
    "owner_uid": 10001,
    "group_name": "技术交流群",
    "member_uids": "10001|10002|10003"
}
```

**响应**:
```json
{
    "type": "create_group_response",
    "status": "success",
    "gid": 20001,
    "group_name": "技术交流群",
    "message": "群组创建成功"
}
```

#### 8. 添加好友

**请求**:
```json
{
    "type": "add_friend",
    "uid": 10001,
    "friend_uid": 10002
}
```

**响应**:

```json
{
    "type": "add_friend_response",
    "status": "success",
    "message": "好友添加成功"
}
```

### 错误码定义

| 错误码 | 说明 |
|--------|------|
| 0 | 成功 |
| 1001 | 用户名已存在 |
| 1002 | 用户不存在 |
| 1003 | 密码错误 |
| 1004 | 用户已在线 |
| 2001 | 群组不存在 |
| 2002 | 群成员已存在 |
| 3001 | 消息格式错误 |
| 3002 | 数据包过大 |
| 4001 | 数据库操作失败 |
| 5001 | 服务器内部错误 |

---

## 📝 日志系统

### 日志宏定义

`header/log.h` 提供了简单的日志宏：

```cpp
// 通用日志
#define LOG_ERROR(msg) \
    fprintf(stderr, "[ERROR] %s:%d (%s) - %s\n", \
            __FILE__, __LINE__, __FUNCTION__, msg);

// 系统错误日志（包含 errno）
#define LOG_PERROR(msg) \
    fprintf(stderr, "[ERROR] %s:%d (%s) - %s: %s\n", \
            __FILE__, __LINE__, __FUNCTION__, msg, strerror(errno));
```

### 使用示例

```cpp
// 记录普通错误
LOG_ERROR("用户认证失败");

// 记录系统调用错误
if (listen(sockfd, 10) < 0) {
    LOG_PERROR("监听 socket 失败");
    return -1;
}

// 记录连接信息
printf("[INFO] 客户端连接: %s:%d\n",
       inet_ntoa(client_addr.sin_addr),
       ntohs(client_addr.sin_port));
```

### 日志建议

**生产环境优化建议**:
1. 使用专业日志库（如 spdlog、log4cpp）
2. 支持日志级别（DEBUG、INFO、WARN、ERROR）
3. 支持日志轮转（按大小或时间）
4. 异步日志（避免阻塞 I/O）
5. 结构化日志（JSON 格式）

**示例使用 spdlog**:

```cpp
#include <spdlog/spdlog.h>

// 初始化日志
auto logger = spdlog::rotating_logger_mt("server",
                                         "logs/server.log",
                                         1024 * 1024 * 10, // 10MB
                                         5);                // 保留 5 个文件

// 使用日志
logger->info("服务器启动，监听端口: {}", port);
logger->error("数据库连接失败: {}", error_msg);
```

---

## 🔧 故障排除

### 常见问题

#### 1. 服务器启动失败

**症状**: `./chatServer` 报错退出

**可能原因**:
- 端口被占用
- MySQL 连接失败
- 权限不足

**解决方案**:
```bash
# 检查端口占用
netstat -tlnp | grep 8888
# 或
lsof -i :8888

# 检查 MySQL 服务
sudo systemctl status mysql
# 或
sudo systemctl status mysqld

# 检查日志
./chatServer 2>&1 | tee server.log
```

#### 2. 客户端无法连接

**症状**: 客户端连接超时

**排查步骤**:
```bash
# 1. 检查服务器是否运行
ps aux | grep chatServer

# 2. 检查端口监听
netstat -tlnp | grep 8888

# 3. 检查防火墙
sudo iptables -L -n
# 或
sudo firewall-cmd --list-ports

# 4. 测试网络连通
telnet <server_ip> 8888
```

**解决方案**:
```bash
# 开放端口（CentOS）
sudo firewall-cmd --permanent --add-port=8888/tcp
sudo firewall-cmd --reload

# 开放端口（Ubuntu）
sudo ufw allow 8888
```

#### 3. 数据库连接失败

**症状**: 日志显示 "MySQL 连接失败"

**排查步骤**:
1. 检查 MySQL 服务状态
2. 验证用户名密码
3. 检查用户权限
4. 验证数据库名

**解决方案**:
```sql
-- 登录 MySQL
mysql -u root -p

-- 创建数据库
CREATE DATABASE chat_system;

-- 创建用户并授权
CREATE USER 'chat_user'@'localhost' IDENTIFIED BY 'password123';
GRANT ALL PRIVILEGES ON chat_system.* TO 'chat_user'@'localhost';
FLUSH PRIVILEGES;

-- 测试连接
mysql -u chat_user -p chat_system
```

#### 4. 内存泄漏

**症状**: 服务器运行时间越长占用内存越多

**排查方法**:
```bash
# 使用 valgrind 检测
valgrind --tool=memcheck ./chatServer

# 使用 top 监控
watch -n 1 'ps aux | grep chatServer'
```

**注意事项**:
- 确保 `bufferevent_free()` 被调用
- 确保数据库连接正确关闭
- 确保智能指针正确使用

---

## 📄 许可证

本项目采用 [MIT](LICENSE) ---许可证。

 👨‍💻 开发者：所说咋咋2035

---

## 🙏 致谢

感谢以下开源项目：

- [libevent](https://libevent.org/) - 高性能网络事件库
- [MySQL](https://www.mysql.com/) - 关系型数据库
- [JsonCpp](https://github.com/open-source-parsers/jsoncpp) - JSON 解析库

---

⭐ **如果这个项目对你有帮助，请给我们一个 Star！** ⭐
