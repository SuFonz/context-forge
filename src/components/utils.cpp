#include <random>
#include <rpc.h>
#include <rpcdce.h>
#include <string>
#include <windows.h>
#include "utils.h"

std::string Gen::random_string(size_t length) {
    const char chars[] = 
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dis(
        0, sizeof(chars) - 2
    );

    std::string result;

    for (size_t i = 0; i < length; i++) {
        result += chars[dis(gen)];
    }

    return result;
}

std::string Gen::random_guid() {
    GUID guid;

    if (FAILED(CoCreateGuid(&guid)))
        return {};

    wchar_t buffer[64];

    if (StringFromGUID2(guid, buffer, 64) == 0)
        return {};

    std::wstring value(buffer);

    return std::string(value.begin(), value.end());
}

std::string Encoding::utf8_to_ansi(const std::string& utf8) {
    if (utf8.empty())
        return {};

    int wideSize = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        utf8.data(),
        static_cast<int>(utf8.size()),
        nullptr,
        0
    );

    if (wideSize <= 0)
        return {};

    std::wstring wide(wideSize, L'\0');

    if (MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            utf8.data(),
            static_cast<int>(utf8.size()),
            wide.data(),
            wideSize
        ) <= 0) {
        return {};
    }

    int ansiSize = WideCharToMultiByte(
        CP_ACP,
        0,
        wide.data(),
        wideSize,
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (ansiSize <= 0)
        return {};

    std::string ansi(ansiSize, '\0');

    if (WideCharToMultiByte(
            CP_ACP,
            0,
            wide.data(),
            wideSize,
            ansi.data(),
            ansiSize,
            nullptr,
            nullptr
        ) <= 0) {
        return {};
    }

    return ansi;
}

std::string Encoding::ansi_to_utf8(const std::string& ansi) {
    if (ansi.empty())
        return {};

    int wideSize = MultiByteToWideChar(
        CP_ACP,
        0,
        ansi.data(),
        static_cast<int>(ansi.size()),
        nullptr,
        0
    );

    if (wideSize <= 0)
        return {};

    std::wstring wide(wideSize, L'\0');

    if (MultiByteToWideChar(
            CP_ACP,
            0,
            ansi.data(),
            static_cast<int>(ansi.size()),
            wide.data(),
            wideSize
        ) <= 0) {
        return {};
    }

    int utf8Size = WideCharToMultiByte(
        CP_UTF8,
        0,
        wide.data(),
        wideSize,
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (utf8Size <= 0)
        return {};

    std::string utf8(utf8Size, '\0');

    if (WideCharToMultiByte(
            CP_UTF8,
            0,
            wide.data(),
            wideSize,
            utf8.data(),
            utf8Size,
            nullptr,
            nullptr
        ) <= 0) {
        return {};
    }

    return utf8;
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter){
    std::vector<std::string> result;
    std::string current;

    for (char ch : str) {
        if (ch == delimiter) {
            result.push_back(current);
            current.clear();
        } else {
            current += ch;
        }
    }

    result.push_back(current);

    return result;
}

std::string StringUtils::join(const std::vector<std::string>& items, const std::string& delimiter) {
    std::string result;

    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0)
            result += delimiter;

        result += items[i];
    }

    return result;
}
