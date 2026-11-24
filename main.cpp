#include <iostream>
#include "order_list.h"
#include "menu.h"

int main() {
    try {
        OrderList list; // Список заказов на всё время работы программы
        Menu menu(list); // Консольный интерфейс программы
        menu.run();
        std::cout << "Exit" << std::endl;
    }
    catch (std::exception &e) {
        std::cout << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
