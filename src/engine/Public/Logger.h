#pragma once

#include <string>
#include <string_view>
#include <cstdio>
#include <format>
#include <source_location>

enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info  = 2,
    Warn  = 3,
    Error = 4,
    Fatal = 5
};

struct SourceLocStr {
    std::string_view value;
    std::source_location loc;

    SourceLocStr(std::string_view s,
                 const std::source_location& l = std::source_location::current())
        : value(s), loc(l) {}

    SourceLocStr(const char* s,
                 const std::source_location& l = std::source_location::current())
        : value(s), loc(l) {}

    SourceLocStr(const std::string& s,
                 const std::source_location& l = std::source_location::current())
        : value(s), loc(l) {}
};

class Logger {
public:
    static void Init();
    static void SetMinLogLevel(LogLevel level);

    static void SetDumpPath(const std::string& path);
    static void SetMinDumpLogLevel(LogLevel level);
    static void SetMinStackTraceLogLevel(LogLevel level);

    template <typename... Args>
    static void Trace(SourceLocStr fmt, Args&&... args) {
        LogV(LogLevel::Trace, fmt.value, std::make_format_args(args...), fmt.loc);
    }

    template <typename... Args>
    static void Debug(SourceLocStr fmt, Args&&... args) {
        LogV(LogLevel::Debug, fmt.value, std::make_format_args(args...), fmt.loc);
    }

    template <typename... Args>
    static void Info(SourceLocStr fmt, Args&&... args) {
        LogV(LogLevel::Info, fmt.value, std::make_format_args(args...), fmt.loc);
    }

    template <typename... Args>
    static void Warn(SourceLocStr fmt, Args&&... args) {
        LogV(LogLevel::Warn, fmt.value, std::make_format_args(args...), fmt.loc);
    }

    template <typename... Args>
    static void Error(SourceLocStr fmt, Args&&... args) {
        LogV(LogLevel::Error, fmt.value, std::make_format_args(args...), fmt.loc);
    }

    template <typename... Args>
    static void Fatal(SourceLocStr fmt, Args&&... args) {
        LogV(LogLevel::Fatal, fmt.value, std::make_format_args(args...), fmt.loc);
    }

private:
    static void LogV(LogLevel level, std::string_view fmt, std::format_args args, const std::source_location& loc);
    static void LogImpl(LogLevel level, std::string&& msg, const std::source_location& loc);

    static const char* LevelPrefix(LogLevel level);
    static LogLevel s_minLogLevel;
    static LogLevel s_minDumpLogLevel;
    static LogLevel s_minStackTraceLogLevel;
    static std::FILE* s_dumpFile;
};
