#include <string>
#include <vector>
#include "menu_item.h"

class Tool {
public:
    Tool() = default;
    ~Tool() = default;

    void load_config(std::string filename = "./config.json");
    void save_config(std::string filename = "./config.json");
private:
    std::string win11_clsid;
    std::vector<MenuItem> win10_menus;
    std::vector<MenuItem> win11_menus;
};