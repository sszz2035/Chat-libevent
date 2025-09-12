#ifndef LOG_H
#define LOG_H
#pragma once

#include <fstream>
#include<iostream>
#include <filesystem>
#include <memory>
#include <mutex>
#include <QString>
#include "utils/get-time/GetCurTime.h"

namespace SSLog {
// 日志级别
enum class LogLevel {
    SS_INFO = 1,
    SS_WARNING = 2,
    SS_ERROR = 3,
    SS_DEFAULT = 4,
};

class LogFile {
public:
    LogFile(const QString& logName);

    ~LogFile();

    void write(const QString& message);
private:
    std::ofstream file;
    std::mutex mutex;
};

inline std::unique_ptr<LogFile> logFile;

// 初始化日志文件
inline void initLogFile(const QString& logName) {
    static std::once_flag flag;
    try {
        std::call_once(flag, [&] {
            logFile = std::make_unique<LogFile>(logName);
        });
    } catch (const std::exception& e) {
        std::cerr << "FATAL: " << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

// 日志函数
void log(LogLevel level, const QString& file, int line, const QString& message);
}

#endif //LOG_H
