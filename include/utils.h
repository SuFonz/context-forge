#include <windows.h>
#include <string>
#include <winnls.h>

class Gen {
public:
    static std::string random_string(size_t length);
    static std::string random_guid();
};

class Encoding {
public:
    // UTF-8 -> ANSI
    static std::string utf8_to_ansi(const std::string& utf8);

    // ANSI -> UTF-8
    static std::string ansi_to_utf8(const std::string& ansi);
};
