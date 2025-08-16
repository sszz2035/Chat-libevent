#ifndef LOG_H
#define LOG_H
#include <iostream>
#include <string>
#include <cerrno>
#include <cstring>
/**
 * @brief 错误报告的宏
 * @note 没有打印错误码
 * @param msg 自定义错误信息
*/
#define LOG_ERROR(msg) \
    do { \
        std::cerr << "[ERROR] " << __FILE__ << ":" << __LINE__ << " in " << __func__ \
                  << " - " << (msg) << std::endl; \
    } while(0)

/**
 * @brief 专门为系统调用错误设计的宏，结合 errno
 * @param msg 自定义错误信息
*/
#define LOG_PERROR(msg) \
    do { \
        std::cerr << "[ERROR] " << __FILE__ << ":" << __LINE__ << " in " << __func__ \
                  << " - " << (msg) << ": " << strerror(errno) << std::endl; \
    } while(0)

#endif