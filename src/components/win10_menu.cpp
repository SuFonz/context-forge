#include <cstddef>
#include <format>
#include <random>
#include <string>
#include <windows.h>
#include <winreg.h>
#include "win10_menu.h"
#include "registry.h"
#include "defs.h"

bool Win10Menu::add_menu(MenuItem item) {
    if (!Registry::exists(HKEY_CURRENT_USER, Registry::SHELL_PATH)) {
        if (!Registry::create(HKEY_CURRENT_USER, Registry::SHELL_PATH)) {
            return false;
        }
    }

    std::string path = std::format("{}\\{}", Registry::SHELL_PATH, item.name);
    std::string command_path = std::format("{}\\{}", path, "command");
    
    if (!Registry::create(HKEY_CURRENT_USER, path)) {
        return false;
    }

    if (!Registry::create(HKEY_CURRENT_USER, command_path)) {
        return false;
    }

    if (!Registry::setString(HKEY_CURRENT_USER, path, std::string(), item.label)) {
        return false;
    }

    if (!Registry::setString(HKEY_CURRENT_USER, command_path, std::string(), item.command)) {
        return false;
    }

    return true;
}

bool Win10Menu::remove_menu(std::string item_name) {
    if (!Registry::exists(HKEY_CURRENT_USER, Registry::SHELL_PATH)) {
        return false;
    }

    std::string path = std::format("{}\\{}", Registry::SHELL_PATH, item_name);

    if (!Registry::exists(HKEY_CURRENT_USER, path)) {
        return false;
    }

    Registry::remove(HKEY_CURRENT_USER, path);

    return true;
}
