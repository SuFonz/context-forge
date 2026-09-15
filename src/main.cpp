#include <format>
#include <iostream>
#include <string>
#include <vector>
#include <CLI/CLI.hpp>

#include "defs.h"
#include "appx.h"
#include "utils.h"
#include "menu_item.h"
#include "win10_menu.h"
#include "win11_menu.h"

// 支持的菜单模式
const std::string MODE_WIN10 = "win10";
const std::string MODE_WIN11 = "win11";

bool appx_package_installed() {
    if (!Appx::is_installed(PROJECT_APPX_PACKAGE_NAME)) {
        std::cout << "Appx package is not installed." << std::endl;
        std::cout << "You can enable Developer Mode and run the following command in PowerShell to install it:" << std::endl;
        std::cout << "Add-AppxPackage -Register .\\AppxManifest.xml" << std::endl;
        std::cout << "You can disable Developer Mode after running this command." << std::endl;
        return false;
    }
    return true;
}

// 根据模式添加菜单项
bool add_entry(const std::string& mode, const MenuItem& item) {
    if (mode == MODE_WIN10)
        return Win10Menu::add_menu(item);

    if (mode == MODE_WIN11 && appx_package_installed())
        return Win11Menu::add_menu(item);

    return false;
}

// 根据模式删除菜单项
bool remove_entry(const std::string& mode, const std::string& name) {
    if (mode == MODE_WIN10)
        return Win10Menu::remove_menu(name);

    if (mode == MODE_WIN11 && appx_package_installed())
        return Win11Menu::remove_menu(name);

    return false;
}

// 根据模式获取菜单项
std::vector<MenuItem> get_entries(const std::string& mode) {
    if (mode == MODE_WIN10)
        return Win10Menu::get_items();

    if (mode == MODE_WIN11 && appx_package_installed())
        return Win11Menu::get_items();

    return {};
}

// 打印菜单项
void print_entries(const std::vector<MenuItem>& items) {
    for (const MenuItem& item : items) {
        std::cout << std::format("name:    {}", item.name) << std::endl;
        std::cout << std::format("label:   {}", item.label) << std::endl;
        std::cout << std::format("command: {}", item.command) << std::endl;
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    CLI::App app{PROJECT_NAME};

    app.require_subcommand(1);

    std::string mode;
    std::string label;
    std::string command;
    std::string target;

    CLI::App* add = app.add_subcommand("add", "Add a context menu entry");
    add->add_option("--mode,-m", mode, "Windows mode")
        ->required()
        ->check(CLI::IsMember({MODE_WIN10, MODE_WIN11}));
    add->add_option("label", label, "Menu label")->required();
    add->add_option("command", command, "Command to execute")->required();

    CLI::App* remove = app.add_subcommand("remove", "Remove a context menu entry");
    remove->add_option("--mode,-m", mode, "Windows mode")
        ->required()
        ->check(CLI::IsMember({MODE_WIN10, MODE_WIN11}));
    remove->add_option("name", target, "Menu label or name")->required();

    CLI::App* list = app.add_subcommand("list", "List context menu entries");
    list->add_option("--mode,-m", mode, "Windows mode")
        ->required()
        ->check(CLI::IsMember({MODE_WIN10, MODE_WIN11}));

    int exit_code = 0;

    add->callback([&]() {
        MenuItem item;
        item.name = std::format("{}_{}", PROJECT_NAME, Gen::random_string(8));
        item.label = label;
        item.command = command;

        if (!add_entry(mode, item)) {
            std::cerr << std::format("Failed to add entry: {}", label) << std::endl;
            exit_code = 1;
        }
    });

    remove->callback([&]() {
        int removed = 0;

        for (const MenuItem& item : get_entries(mode)) {
            if (item.name != target)
                continue;

            if (remove_entry(mode, item.name))
                ++removed;
        }

        if (removed == 0) {
            std::cerr << std::format("Failed to remove entry: {}", target) << std::endl;
            exit_code = 1;
        }
    });

    list->callback([&]() {
        print_entries(get_entries(mode));
    });

    CLI11_PARSE(app, argc, argv);

    return exit_code;
}
