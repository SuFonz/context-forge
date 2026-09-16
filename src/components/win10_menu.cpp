#include <algorithm>
#include <format>
#include <iterator>
#include <optional>
#include <string>
#include <vector>
#include <windows.h>
#include <winreg.h>
#include "win10_menu.h"
#include "registry.h"
#include "defs.h"
#include "utils.h"

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

    std::string command = item.args.empty()
        ? std::format("{}", item.program)
        : std::format("{} {}", item.program, StringUtils::join(item.args, " "));

    if (!Registry::setString(HKEY_CURRENT_USER, command_path, std::string(), command)) {
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

std::vector<MenuItem> Win10Menu::get_items() {
    if (!Registry::exists(HKEY_CURRENT_USER, Registry::SHELL_PATH)) {
        return {};
    }

    std::vector<std::string> sub_keys = Registry::getSubKeys(HKEY_CURRENT_USER, Registry::SHELL_PATH);

    std::erase_if(sub_keys, [](const std::string& key) {
        return !key.starts_with(PROJECT_NAME);
    });

    std::vector<MenuItem> items;

    std::transform(
        sub_keys.begin(),
        sub_keys.end(),
        std::back_inserter(items),
        [](const std::string& key) {
            std::string path = std::format("{}\\{}", Registry::SHELL_PATH, key);
            std::string cmd_path = std::format("{}\\command", path);

            MenuItem item;
            item.name = key;

            std::optional<std::string> label = Registry::getString(HKEY_CURRENT_USER, path, {});

            if (label)
                item.label = *label;

            if (Registry::exists(HKEY_CURRENT_USER, cmd_path)) {
                std::optional<std::string> cmd = Registry::getString(HKEY_CURRENT_USER, cmd_path, {});

                if (cmd) {
                    std::string command = *cmd;

                    std::vector<std::string> tokens = StringUtils::split(command, ' ');
                    std::vector<std::string> parts;

                    // 双引号内部的空格不作为分隔符，
                    // 例如 "C:\Program Files\program.exe" "%1" "%2"
                    for (const std::string& token : tokens) {
                        if (!parts.empty() && parts.back().starts_with('"') && !parts.back().ends_with('"')) {
                            parts.back() += " " + token;
                        } else if (!token.empty()) {
                            parts.push_back(token);
                        }
                    }

                    if (!parts.empty()) {
                        item.program = parts.front();
                        item.args.assign(parts.begin() + 1, parts.end());
                    }
                }

            }

            return item;
        }
    );

    return items;
}
