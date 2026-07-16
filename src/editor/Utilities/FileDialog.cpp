#include "FileDialog.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shobjidl.h>
#include <vector>

static std::wstring ToWide(const char* utf8) {
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    std::wstring wstr(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &wstr[0], len);
    return wstr;
}

static std::string ToUTF8(const wchar_t* wide) {
    int len = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 1) return {};
    std::string str(static_cast<size_t>(len) - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, &str[0], len, nullptr, nullptr);
    return str;
}

static LONG WINAPI P9npExceptionFilter(EXCEPTION_POINTERS* ep) {
    static HMODULE p9np = GetModuleHandleW(L"p9np.dll");
    if (!p9np) return EXCEPTION_CONTINUE_SEARCH;

    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(ep->ExceptionRecord->ExceptionAddress, &mbi, sizeof(mbi)))
        return EXCEPTION_CONTINUE_SEARCH;

    if (mbi.AllocationBase == reinterpret_cast<void*>(p9np))
        return EXCEPTION_CONTINUE_EXECUTION;

    return EXCEPTION_CONTINUE_SEARCH;
}

std::string FileDialog::OpenFile(const char* title, const char* filter, void* parentHwnd) {
    IFileOpenDialog* pfd = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));
    if (FAILED(hr) || !pfd) return {};

    DWORD dwFlags = 0;
    pfd->GetOptions(&dwFlags);
    pfd->SetOptions(dwFlags | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST);

    if (title && *title) {
        pfd->SetTitle(ToWide(title).c_str());
    }

    std::vector<std::wstring> names;
    std::vector<std::wstring> patterns;
    std::vector<COMDLG_FILTERSPEC> specs;

    if (filter && *filter) {
        const char* p = filter;
        while (*p) {
            const char* nameStart = p;
            p += strlen(p) + 1;
            if (!*p) break;

            const char* patternStart = p;
            p += strlen(p) + 1;

            names.push_back(ToWide(nameStart));
            patterns.push_back(ToWide(patternStart));
        }

        for (size_t i = 0; i < names.size(); ++i) {
            COMDLG_FILTERSPEC spec;
            spec.pszName = names[i].c_str();
            spec.pszSpec = patterns[i].c_str();
            specs.push_back(spec);
        }
    }

    if (!specs.empty()) {
        pfd->SetFileTypes(static_cast<UINT>(specs.size()), specs.data());
    }

    PVOID veh = AddVectoredExceptionHandler(1, P9npExceptionFilter);

    std::string result;
    hr = pfd->Show(static_cast<HWND>(parentHwnd));

    RemoveVectoredExceptionHandler(veh);

    if (SUCCEEDED(hr)) {
        IShellItem* psi = nullptr;
        hr = pfd->GetResult(&psi);
        if (SUCCEEDED(hr) && psi) {
            PWSTR path = nullptr;
            hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &path);
            if (SUCCEEDED(hr) && path) {
                result = ToUTF8(path);
                CoTaskMemFree(path);
            }
            psi->Release();
        }
    }

    pfd->Release();
    return result;
}

std::string FileDialog::OpenFolder(const char* title, void* parentHwnd) {
    IFileOpenDialog* pfd = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));
    if (FAILED(hr) || !pfd) return {};

    DWORD dwFlags = 0;
    pfd->GetOptions(&dwFlags);
    pfd->SetOptions(dwFlags | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);

    if (title && *title) {
        pfd->SetTitle(ToWide(title).c_str());
    }

    PVOID veh = AddVectoredExceptionHandler(1, P9npExceptionFilter);

    std::string result;
    hr = pfd->Show(static_cast<HWND>(parentHwnd));

    RemoveVectoredExceptionHandler(veh);

    if (SUCCEEDED(hr)) {
        IShellItem* psi = nullptr;
        hr = pfd->GetResult(&psi);
        if (SUCCEEDED(hr) && psi) {
            PWSTR path = nullptr;
            hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &path);
            if (SUCCEEDED(hr) && path) {
                result = ToUTF8(path);
                CoTaskMemFree(path);
            }
            psi->Release();
        }
    }

    pfd->Release();
    return result;
}
