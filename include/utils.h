#include <vector>
#include <windows.h>
#include <string>
#include <winnls.h>

class Gen {
public:
    Gen() = delete;

    // 生成随机字符串
    static std::string random_string(size_t length);

    // 生成 guid
    static std::string random_guid();
};

class Encoding {
public:
    Encoding() = delete;

    // UTF-8 -> ANSI
    static std::string utf8_to_ansi(const std::string& utf8);

    // ANSI -> UTF-8
    static std::string ansi_to_utf8(const std::string& ansi);
};

class StringUtils {
public:
    StringUtils() = delete;

    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string join(const std::vector<std::string>& items, const std::string& delimiter);
};
