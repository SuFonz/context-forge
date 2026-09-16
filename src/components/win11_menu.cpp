#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <windows.h>
#include <nlohmann/json.hpp>
#include "win11_menu.h"
#include "menu_item.h"

// 配置目录：模块（EXE / DLL）所在目录，保证不受当前工作目录影响
std::filesystem::path Win11Menu::config_dir() {
    HMODULE module = nullptr;

    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&Win11Menu::CONFIG_FILE),
        &module
    );

    std::wstring buffer(MAX_PATH, L'\0');

    DWORD length = GetModuleFileNameW(module, buffer.data(), static_cast<DWORD>(buffer.size()));

    if (length == 0 || length >= buffer.size())
        return std::filesystem::current_path();

    buffer.resize(length);

    return std::filesystem::path(buffer).parent_path();
}

std::filesystem::path Win11Menu::config_path() {
    return config_dir() / CONFIG_FILE;
}

bool Win11Menu::add_menu(MenuItem item) {
    std::filesystem::path path = config_path();

    std::vector<MenuItem> menu_items = items_from_json(read_config(path));

    menu_items.push_back(item);

    write_config(path, menu_items);

    return true;
}

bool Win11Menu::remove_menu(std::string name) {
    std::filesystem::path path = config_path();

    std::vector<MenuItem> menu_items = items_from_json(read_config(path));

    std::erase_if(menu_items, [&](const MenuItem& item) -> bool {
        return item.name == name;
    });

    write_config(path, menu_items);

    return true;
}

std::vector<MenuItem> Win11Menu::get_items() {
    std::filesystem::path path = config_path();

    if (!std::filesystem::exists(path))
        return {};

    return items_from_json(read_config(path));
}

std::string Win11Menu::read_config(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file)
        return {};

    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

void Win11Menu::write_config(const std::filesystem::path& path, std::vector<MenuItem> menu_items) {
    nlohmann::json data;

    data["items"] = nlohmann::json::array();

    for (const auto& mi : menu_items) {
        data["items"].push_back({
            {"name", mi.name},
            {"label", mi.label},
            {"program", mi.program},
            {"args", mi.args},
        });
    }

    std::ofstream file(path, std::ios::binary);

    file << data.dump(4);
}

std::vector<MenuItem> Win11Menu::items_from_json(std::string json_text) {
    std::vector<MenuItem> menu_items;

    nlohmann::json data;

    if (json_text.empty()) {
        return std::vector<MenuItem>();
    }

    try {
        data = nlohmann::json::parse(json_text);

        for (const auto& item : data["items"]) {
            MenuItem mn_item = {
                .name = item["name"],
                .label = item["label"],
                .program = item["program"],
                .args = item["args"],
            };

            menu_items.push_back(mn_item);
        }

    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return std::vector<MenuItem>();
    }

    return menu_items;
}
