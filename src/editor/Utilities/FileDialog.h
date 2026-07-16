#pragma once

#include <string>

namespace FileDialog {

std::string OpenFile(const char* title, const char* filter, void* parentHwnd = nullptr);
std::string OpenFolder(const char* title, void* parentHwnd = nullptr);

}
