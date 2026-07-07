#include "Logger.h"
#include "Utils.h"

#include <cstdio>
#include <stacktrace>
#include <Windows.h>

LogLevel Logger::s_minLogLevel = LogLevel::Trace;
LogLevel Logger::s_minDumpLogLevel = LogLevel::Info;
LogLevel Logger::s_minStackTraceLogLevel = LogLevel::Warn;
std::FILE* Logger::s_dumpFile = nullptr;

void Logger::Init() {
    if (!IS_DEBUG) {
        s_minLogLevel = LogLevel::Warn;
    }

    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(handle, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(handle, mode);

    SetDumpPath("log.txt");
}

void Logger::SetMinLogLevel(LogLevel level) {
    s_minLogLevel = level;
}

void Logger::SetDumpPath(const std::string& path) {
    if (s_dumpFile) {
        fclose(s_dumpFile);
    }
    fopen_s(&s_dumpFile, path.c_str(), "a");
    if (s_dumpFile) {
        fprintf(s_dumpFile, "--- Log started ---\n");
    }
}

void Logger::SetMinDumpLogLevel(LogLevel level) {
    s_minDumpLogLevel = level;
}

void Logger::SetMinStackTraceLogLevel(LogLevel level) {
    s_minStackTraceLogLevel = level;
}

const char* Logger::LevelPrefix(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "[TRACE]";
        case LogLevel::Debug: return "[DEBUG]";
        case LogLevel::Info:  return "[INFO]";
        case LogLevel::Warn:  return "[WARN]";
        case LogLevel::Error: return "[ERROR]";
        case LogLevel::Fatal: return "[FATAL]";
    }
    return "[?]";
}

void Logger::LogV(LogLevel level, std::string_view fmt, std::format_args args, const std::source_location& loc) {
    LogImpl(level, std::vformat(fmt, args), loc);
}

void Logger::LogImpl(LogLevel level, std::string&& msg, const std::source_location& loc) {
    std::string_view filepath(loc.file_name());
    auto sepPos = filepath.find_last_of("/\\");
    std::string_view filename = (sepPos != std::string_view::npos) ? filepath.substr(sepPos + 1) : filepath;
    std::string text = std::format("{} {} [{}:{}] {}", Utils::CurrentTimeString(), LevelPrefix(level), filename, loc.line(), msg);

    if (level >= s_minStackTraceLogLevel) {
        text = std::format("-----------TraceBack Begin-----------\n"
                           "{}\n"
                           "StackTrace:\n"
                           "{}\n"
                           "-----------TraceBack End-------------", text, std::stacktrace::current());
    }

    if (level >= s_minLogLevel) {
        const char* color = "";
        switch (level) {
            case LogLevel::Trace: color = "\033[90m"; break;
            case LogLevel::Debug: color = "\033[36m"; break;
            case LogLevel::Info:  color = "\033[32m"; break;
            case LogLevel::Warn:  color = "\033[33m"; break;
            case LogLevel::Error: color = "\033[31m"; break;
            case LogLevel::Fatal: color = "\033[35m"; break;
        }
        std::puts(std::format("{}{}\033[0m", color, text).c_str());
    }

    if (level >= s_minDumpLogLevel && s_dumpFile) {
        fprintf(s_dumpFile, "%s\n", text.c_str());
        fflush(s_dumpFile);
    }
}
