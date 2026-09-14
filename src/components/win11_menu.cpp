#include <format>
#include <rpc.h>
#include <rpcdce.h>
#include <string>
#include <windows.h>
#include <winreg.h>
#include "win11_menu.h"
#include "registry.h"

std::string Win11Menu::register_clsid() {
    std::string uuid = "{00000000-0000-0000-0000-000000000001}";

    std::string path = std::format("{}\\{}", Registry::CLSID_PATH, uuid);

    if (Registry::exists(HKEY_CURRENT_USER, path)) {
        return std::string();
    }

    if (!Registry::create(HKEY_CURRENT_USER, path)) {
        return std::string();
    }

    return uuid;
}

bool Win11Menu::add_menu(MenuItem item) {

}

bool Win11Menu::remove_menu(std::string name) {

}
