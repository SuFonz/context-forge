#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "menu_item.h"

class Win11Menu {
public:
    static bool add_menu(MenuItem item);
    static bool remove_menu(std::string name);
    static std::vector<MenuItem> get_items();
    inline static const char* CONFIG_FILE = "win11_menu.json";
protected:
    static std::filesystem::path config_dir();
    static std::filesystem::path config_path();
    static std::string read_config(const std::filesystem::path& path);
    static void write_config(const std::filesystem::path& path, std::vector<MenuItem> menu_items);
    static std::vector<MenuItem> items_from_json(std::string json_text);
};
