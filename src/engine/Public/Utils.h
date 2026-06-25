#pragma once

#include <string>

#ifdef _DEBUG
    constexpr bool IS_DEBUG = true;
#else
    constexpr bool IS_DEBUG = false;
#endif

class Utils {
public:
    static std::string CurrentTimeString();
};
