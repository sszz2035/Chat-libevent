//
// Created by FUQAQ on 2025/1/14.
//

#include <iostream>
#include <qDebug>
#include "utils/log/logfile.h"
namespace SSLog {
LogFile::LogFile(const QString &logName) {
    std::filesystem::path dir = "log";
    if (!std::filesystem::exists(dir)) {
        try {
            if (!std::filesystem::create_directories(dir)) {
                throw std::runtime_error("Failed to create log directory");
            }
            qDebug() << "Log directory created successfully:" << QString::fromStdString(dir.string());
        } catch (const std::exception& e) {
            qDebug() << "Error creating log directory:" << e.what();
            throw;
        }
    }
    
    std::string filename = "log/" + logName.toStdString() + "_" + GetCurTime::getTimeObj()->getCurTime("%Y-%m-%d") + ".log";
    qDebug() << "Attempting to open log file:" << QString::fromStdString(filename);
    
    file.open(filename, std::ios::out | std::ios::app);
    if (!file.is_open()) {
        qDebug() << "Failed to open log file. Current working directory:" << QString::fromStdString(std::filesystem::current_path().string());
        throw std::runtime_error("Failed to open log file: " + filename);
    }
    qDebug() << "Log file opened successfully:" << QString::fromStdString(filename);
}

LogFile::~LogFile() {
    if (file.is_open()) {
        file.close();
    }
}

void LogFile::write(const QString &message)  {
    std::lock_guard<std::mutex> lock(mutex);
    if (file.is_open()) {
        qDebug() << "Writing to log file:" << message;
        file << message.toStdString() << std::endl;
        file.flush();
        qDebug() << "Log write completed, file good:" << file.good();
    } else {
        qDebug() << "Cannot write to log file - file is not open";
    }
}

void log(LogLevel level, const QString &file, int line, const QString &message)  {
    QString levelStr;
    switch (level) {
    case LogLevel::SS_INFO: levelStr = "INFO"; break;
    case LogLevel::SS_WARNING: levelStr = "WARNING"; break;
    case LogLevel::SS_ERROR: levelStr = "ERROR"; break;
    }

    QString logMessage = "[" + levelStr + "] [" + file + "] |in " + QString::number(line) + " line |at " + QString::fromStdString(GetCurTime::getTimeObj()->getCurTime("%Y-%m-%d %H:%M:%S")) + "] : " + message;
    
    qDebug() << "Log function called - level:" << levelStr << "logFile exists:" << (logFile != nullptr);
    
    if (level == LogLevel::SS_DEFAULT) {
        qDebug() << logMessage;
    } else {
        // 输出到日志文件
        if (logFile) {
            try {
                qDebug() << "Attempting to write to log file:" << logMessage;
                logFile->write(logMessage);
                qDebug() << "Log file write successful";
            } catch (const std::exception& e) {
                qDebug() << "Failed to write to log file, falling back to console:" << e.what();
                qDebug() << logMessage;
            }
        } else {
            qDebug() << "Log file not initialized, outputting to console:" << logMessage;
        }
    }
}
}
