#pragma once

#include <string>
#include <cstdio>

enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info  = 2,
    Warn  = 3,
    Error = 4,
    Fatal = 5
};

class Logger {
public:
    static void Init();
    static void SetMinLogLevel(LogLevel level);

    static void SetDumpPath(const std::string& path);
    static void SetMinDumpLogLevel(LogLevel level);
    static void SetMinStackTraceLogLevel(LogLevel level);

    static void Trace(const std::string& msg);
    static void Debug(const std::string& msg);
    static void Info(const std::string& msg);
    static void Warn(const std::string& msg);
    static void Error(const std::string& msg);
    static void Fatal(const std::string& msg);

private:
    static void Log(LogLevel level, const std::string& msg);
    static const char* LevelPrefix(LogLevel level);
    static LogLevel s_minLogLevel;
    static LogLevel s_minDumpLogLevel;
    static LogLevel s_minStackTraceLogLevel;
    static std::FILE* s_dumpFile;
};
