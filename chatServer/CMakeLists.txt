cmake_minimum_required(VERSION 2.8)

project(chatServer)

add_executable(${PROJECT_NAME} src/main.cpp src/chat_server.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/header)

target_link_libraries(${PROJECT_NAME} event)

