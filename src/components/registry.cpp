#include "registry.h"

bool Registry::exists(HKEY root, const std::string& path) {
    HKEY key;

    LONG ret = RegOpenKeyExA(root, path.c_str(), 0, KEY_READ, &key);

    if (ret == ERROR_SUCCESS) {
        RegCloseKey(key);
        return true;
    }

    return false;
}

bool Registry::create(HKEY root, const std::string& path) {
    HKEY key;

    LONG ret = RegCreateKeyExA(root, path.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr);

    if (ret != ERROR_SUCCESS)
        return false;

    RegCloseKey(key);

    return true;
}

bool Registry::setString(HKEY root, const std::string& path, const std::string& name, const std::string& value) {
    HKEY key;

    if (RegCreateKeyExA(root, path.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS) {
        return false;
    }

    LONG ret = RegSetValueExA(key, name.empty() ? nullptr : name.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()), value.size()+1);

    RegCloseKey(key);

    return ret == ERROR_SUCCESS;
}

std::optional<std::string> Registry::getString(HKEY root, const std::string& path, const std::string& name) {
    HKEY key;

    if (RegOpenKeyExA(root, path.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return {};
    }

    char buffer[1024];

    DWORD size = sizeof(buffer);

    LONG ret = RegQueryValueExA(key, name.c_str(), nullptr, nullptr, reinterpret_cast<BYTE*>(buffer), &size);

    RegCloseKey(key);

    if (ret != ERROR_SUCCESS)
        return {};

    return std::string(buffer);
}

bool Registry::setDWORD(HKEY root, const std::string& path, const std::string& name, DWORD value) {
    HKEY key;

    if (RegCreateKeyExA(root, path.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS) {
        return false;
    }

    LONG ret = RegSetValueExA(key, name.c_str(), 0, REG_DWORD, reinterpret_cast<BYTE*>(&value), sizeof(value));

    RegCloseKey(key);

    return ret == ERROR_SUCCESS;
}

bool Registry::remove(HKEY root, const std::string& path) {
    return RegDeleteTreeA(root, path.c_str()) == ERROR_SUCCESS;
}

std::vector<std::string> Registry::getSubKeys(HKEY root, const std::string& path) {
    std::vector<std::string> keys;

    HKEY hKey;

    if (RegOpenKeyExA(
            root,
            path.c_str(),
            0,
            KEY_READ,
            &hKey) != ERROR_SUCCESS) {
        return keys;
    }

    DWORD index = 0;
    char name[256];
    DWORD nameSize;

    while (true) {
        nameSize = sizeof(name);

        LONG result = RegEnumKeyExA(
            hKey,
            index,
            name,
            &nameSize,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );

        if (result == ERROR_NO_MORE_ITEMS)
            break;

        if (result == ERROR_SUCCESS) {
            keys.emplace_back(name, nameSize);
        }

        ++index;
    }

    RegCloseKey(hKey);

    return keys;
}
