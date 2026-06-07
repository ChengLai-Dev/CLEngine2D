#include "Utils.h"

#include <format>
#include <ctime>

std::string Utils::CurrentTimeString() {
    std::time_t now = std::time(nullptr);
    std::tm local;
    localtime_s(&local, &now);
    char buf[32];
    strftime(buf, sizeof(buf), "%H:%M:%S", &local);
    return std::format("[{}]", buf);
}
