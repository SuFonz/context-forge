#include <string>
#include <vector>
#include "menu_item.h"

class Win10Menu {
public:
    static bool add_menu(MenuItem item);
    static bool remove_menu(std::string name);
    static std::vector<MenuItem> get_items();
protected:
    
};