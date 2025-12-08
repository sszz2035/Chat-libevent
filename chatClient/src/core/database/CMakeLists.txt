cmake_minimum_required(VERSION 3.19)

# 数据库模块
set(DATABASE_SOURCES
    databasemanager.cpp
    userdao.cpp
    friendshipdao.cpp
    groupdao.cpp
    dbcachehelper.cpp
)

set(DATABASE_HEADERS
    databasemanager.h
    userdao.h
    friendshipdao.h
    groupdao.h
    dbcachehelper.h
)

# 创建库
add_library(DatabaseLib STATIC ${DATABASE_SOURCES} ${DATABASE_HEADERS})

# 设置目标属性
set_target_properties(DatabaseLib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 链接Qt6模块
target_link_libraries(DatabaseLib PRIVATE
    Qt6::Gui
    Qt6::Core
    Qt6::Sql
    Qt6::Network
    Qt6::Widgets
    ${CMAKE_SOURCE_DIR}/lib/ElaWidgetToolsd.lib
)

# 添加包含目录 - 包含core目录以便找到pagedata.h等依赖
target_include_directories(DatabaseLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)
