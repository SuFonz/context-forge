#pragma once

#include <string>

class Appx {
public:
    // 检测指定的 Appx 包是否已安装（等价于 Get-AppxPackage -Name）
    static bool is_installed(const std::string& name);
};
