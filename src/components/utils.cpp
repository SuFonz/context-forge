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
