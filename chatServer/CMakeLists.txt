cmake_minimum_required(VERSION 3.13)

project(chatServer)

# 设置启用C++17
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义可执行文件
add_executable(${PROJECT_NAME} 
src/main.cpp 
src/chat_server.cpp
src/chat_info.cpp 
src/data_base.cpp
src/chat_thread.cpp
src/snowflake_id_generator.cpp)

# pkg-config 查找MySQL库
find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_MYSQL REQUIRED mysqlclient)

# 绑定头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE 
${PROJECT_SOURCE_DIR}/header
${PC_MYSQL_INCLUDE_DIRS})

# message(STATUS "MySQL Found: ${PC_MYSQL_FOUND}")
# message(STATUS "MySQL Include Dirs: ${PC_MYSQL_INCLUDE_DIRS}")
# message(STATUS "MySQL Libraries: ${PC_MYSQL_LIBRARIES}")

#设置库链接目录
target_link_directories(${PROJECT_NAME} PRIVATE
    ${PC_MYSQL_LIBRARY_DIRS}
)

# 链接库
target_link_libraries(${PROJECT_NAME} 
    PRIVATE
    event 
    mysqlclient
    pthread
    jsoncpp
) 