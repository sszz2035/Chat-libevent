CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

# 插件名称
SET(
    PLUGINS_NAME                 LLMChatPagePlugin
)

set(
    CMAKE_AUTOMOC                ON
)
set(
    CMAKE_AUTOUIC                ON
)
set(
    CMAKE_AUTORCC                ON
)

IF      (WIN32)
        SET(CUR_PLATFORM             Win)
ELSEIF  (LINUX)
        SET(CUR_PLATFORM             Linux)
ENDIF   ()

PROJECT(
        ${PLUGINS_NAME}              LANGUAGES                  CXX
)

FIND_PACKAGE(Qt6
        REQUIRED                     COMPONENTS
        Core                         Gui                        Widgets
        Network
)

# qt_add_executable(${PLUGINS_NAME}
#         LLMChatPage.h
#         LLMChatPage.qrc
# )

# 头文件路径
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

# 收集源文件
FILE(
        GLOB_RECURSE                 SRC_CPP                    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# 收集头文件（让Qt Creator显示）
FILE(
        GLOB_RECURSE                 SRC_H                      ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# 收集资源文件（让Qt Creator显示）
FILE(
        GLOB_RECURSE                 SRC_QRC                    ${CMAKE_CURRENT_SOURCE_DIR}/*.qrc
)

ADD_LIBRARY(
        ${PLUGINS_NAME}              SHARED
        ${SRC_CPP}
        ${SRC_H}
        ${SRC_QRC}
)

SET_TARGET_PROPERTIES(
        ${PLUGINS_NAME}              PROPERTIES
        OUTPUT_NAME                  ${PLUGINS_NAME}
        OUTPUT_NAME_DEBUG            ${PLUGINS_NAME}d
        OUTPUT_NAME_RELEASE          ${PLUGINS_NAME}
        CXX_STANDARD                 17
        CXX_STANDARD_REQUIRED        ON
)

# 添加预处理器定义
TARGET_COMPILE_DEFINITIONS(
        ${PLUGINS_NAME}              PRIVATE
        LLMCHAT_PAGE_EXPORTS
)

# 链接库
TARGET_LINK_LIBRARIES(
        ${PLUGINS_NAME}              PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
)

# 链接 ElaWidget 工具库
TARGET_LINK_LIBRARIES(
        ${PLUGINS_NAME}              PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/ElaWidgetToolsd.lib
)
