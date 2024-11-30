#include "Logger.h"
#ifdef _WIN32
#include <windows.h>
#endif

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : consoleLoggingEnabled(true) {
#ifdef _WIN32
    enableANSIColors();
#endif
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> guard(logMutex);
    if (logFile.is_open()) {
        logFile.close();
    }
    logFile.open(filename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

void Logger::enableConsoleLogging(bool enable) {
    std::lock_guard<std::mutex> guard(logMutex);
    consoleLoggingEnabled = enable;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> guard(logMutex);
    std::string logMessage = "[" + getCurrentTime() + "] [" + logLevelToString(level) + "] " + message + "\n";
    std::string colorCode = getColorCode(level);

    if (consoleLoggingEnabled) {
        std::cout << colorCode << logMessage << "\033[0m";
    }
    if (logFile.is_open()) {
        logFile << logMessage;
    }
}

std::string Logger::getCurrentTime() {
    std::time_t now = std::time(0);
    char buffer[80];
    struct tm timeInfo;
    localtime_s(&timeInfo, &now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
    return std::string(buffer);
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARNING:
        return "WARNING";
    case LogLevel::Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

std::string Logger::getColorCode(LogLevel level) {
    switch (level) {
    case LogLevel::INFO:
        return "\033[32m"; // Green
    case LogLevel::WARNING:
        return "\033[33m"; // Yellow
    case LogLevel::Error:
        return "\033[31m"; // Red
    default:
        return "\033[37m"; // Default (White)
    }
}

void Logger::enableANSIColors() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}
