#include "Logger.h"
#include "Utils.h"

#include <format>
#include <cstdio>
#include <Windows.h>

LogLevel Logger::s_minLogLevel = LogLevel::Trace;
LogLevel Logger::s_minDumpLogLevel = LogLevel::Warn;
std::FILE* Logger::s_dumpFile = nullptr;

void Logger::Init() {
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

void Logger::Trace(const std::string& msg) { Log(LogLevel::Trace, msg); }
void Logger::Debug(const std::string& msg) { Log(LogLevel::Debug, msg); }
void Logger::Info(const std::string& msg)  { Log(LogLevel::Info, msg); }
void Logger::Warn(const std::string& msg)  { Log(LogLevel::Warn, msg); }
void Logger::Error(const std::string& msg) { Log(LogLevel::Error, msg); }
void Logger::Fatal(const std::string& msg) { Log(LogLevel::Fatal, msg); }

void Logger::Log(LogLevel level, const std::string& msg) {
    std::string text = std::format("{} {} {}", Utils::CurrentTimeString(), LevelPrefix(level), msg);

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
