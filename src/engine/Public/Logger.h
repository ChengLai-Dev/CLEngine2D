#pragma once

#include <string>
#include <string_view>
#include <cstdio>
#include <format>

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

    template <typename... Args>
    static void Trace(std::string_view fmt, Args&&... args) { LogVFmt(LogLevel::Trace, fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    static void Debug(std::string_view fmt, Args&&... args) { LogVFmt(LogLevel::Debug, fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    static void Info(std::string_view fmt, Args&&... args)  { LogVFmt(LogLevel::Info, fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    static void Warn(std::string_view fmt, Args&&... args)  { LogVFmt(LogLevel::Warn, fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    static void Error(std::string_view fmt, Args&&... args) { LogVFmt(LogLevel::Error, fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    static void Fatal(std::string_view fmt, Args&&... args) { LogVFmt(LogLevel::Fatal, fmt, std::forward<Args>(args)...); }

private:
    static void LogV(LogLevel level, std::string_view fmt, std::format_args args);
    static void LogImpl(LogLevel level, std::string&& msg);

    template <typename... Args>
    static void LogVFmt(LogLevel level, std::string_view fmt, Args&&... args) {
        LogV(level, fmt, std::make_format_args(args...));
    }

    static const char* LevelPrefix(LogLevel level);
    static LogLevel s_minLogLevel;
    static LogLevel s_minDumpLogLevel;
    static LogLevel s_minStackTraceLogLevel;
    static std::FILE* s_dumpFile;
};
