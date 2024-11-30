#pragma once
#include <iostream>
#include <fstream>
#include <mutex>
#include <ctime>
#include <string>

enum class LogLevel {
    INFO,
    WARNING,
    Error
};

class Logger {
public:
    static Logger& getInstance();
    void setLogFile(const std::string& filename);
    void enableConsoleLogging(bool enable);
    void log(LogLevel level, const std::string& message);

private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string getCurrentTime();
    std::string logLevelToString(LogLevel level);
    std::string getColorCode(LogLevel level);
    void enableANSIColors();

    std::mutex logMutex;
    std::ofstream logFile;
    bool consoleLoggingEnabled;
};