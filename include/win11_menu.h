#include <string>
#include "menu_item.h"

class Win11Menu {
public:
    static std::string register_clsid();
    static bool add_menu(MenuItem item);
    static bool remove_menu(std::string name);
protected:
    
};
