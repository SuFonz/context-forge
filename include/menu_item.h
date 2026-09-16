#pragma once

#include <string>
#include <vector>

struct MenuItem {
    std::string name;
    std::string label;
    std::string program; // 包含双引号 ""
    std::vector<std::string> args; // 每个参数都包含双引号 ""
};
