#pragma once

#include <windows.h>
#include <string>
#include <optional>


class Registry {
public:

    // 判断 Key 是否存在
    static bool exists(HKEY root, const std::string& path);

    // 创建 Key
    static bool create(HKEY root, const std::string& path);

    // 设置字符串
    static bool setString(HKEY root, const std::string& path, const std::string& name, const std::string& value);

    // 获取字符串
    static std::optional<std::string> getString(HKEY root, const std::string& path, const std::string& name);

    // 设置 DWORD
    static bool setDWORD(HKEY root, const std::string& path, const std::string& name, DWORD value);

    // 获取 DWORD
    static std::optional<DWORD> getDWORD(HKEY root, const std::string& path, const std::string& name);

    // 删除值
    static bool removeValue(HKEY root, const std::string& path, const std::string& name);

    // 删除 Key（包括子项）
    static bool remove(HKEY root, const std::string& path);

    // Shell 目录
    inline static const char* SHELL_PATH = R"(Software\Classes\*\shell)";

    // CLSID 目录
    inline static const char* CLSID_PATH = R"(Software\Classes\CLSID)";
};
