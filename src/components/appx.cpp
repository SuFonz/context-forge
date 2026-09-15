#include <cstring>
#include <string>
#include <windows.h>
#include <winrt/base.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Management.Deployment.h>
#include "appx.h"

// UTF-8 转 UTF-16
static std::wstring to_wide(const std::string& value) {
    if (value.empty())
        return {};

    int size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);

    if (size <= 0)
        return {};

    std::wstring result(size, L'\0');

    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, result.data(), size);

    result.resize(size - 1);

    return result;
}

bool Appx::is_installed(const std::string& name) {
    if (name.empty())
        return false;

    std::wstring target = to_wide(name);

    try {
        winrt::init_apartment();
    } catch (const winrt::hresult_error&) {
        // 已初始化（模式不同）时忽略，继续查询
    }

    try {
        winrt::Windows::Management::Deployment::PackageManager manager;

        for (const auto& package : manager.FindPackagesForUser(L"")) {
            if (_wcsicmp(package.Id().Name().c_str(), target.c_str()) == 0)
                return true;
        }
    } catch (const winrt::hresult_error&) {
        return false;
    }

    return false;
}
