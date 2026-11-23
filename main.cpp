#include <iostream>
#include <string>
#include "order.h"
#include "order_list.h"
#include "order_exception.h"

// Запрос строки у пользователя
static std::string ask_line(std::string prompt) {
    std::cout << prompt;
    std::string s;
    if (!std::getline(std::cin, s)) {
        throw std::runtime_error("Input stream closed");
    }
    return s;
}

// Запрос числа из диапазона
static int ask_int_in_range(std::string prompt, int min_value, int max_value) {
    while (true) {
        std::string s = ask_line(prompt);
        try {
            size_t pos = 0; // Позиция, на которой завершился разбор
            int value = std::stoi(s, &pos);
            if (pos == s.size() && value >= min_value && value <= max_value) {
                return value;
            }
        }
        catch (std::logic_error&) {}
        std::cout << "Incorrect value, enter a number from " << min_value
                  << " to " << max_value << std::endl;
    }
}

// Ввод суммы с проверкой формата
static double ask_amount() {
    while (true) {
        std::string s = ask_line("Amount (RUB): "); // Введённая сумма
        try {
            size_t pos = 0;
            double value = std::stod(s, &pos);
            if (pos == s.size()) return value;
        }
        catch (std::logic_error&) {}
        std::cout << "Error: amount is not a valid number" << std::endl;
    }
}

// Ввод заказа по полям
static void read_order(Order &order) {
    while (true) {
        try {
            order.set_payer_account(ask_line("Payer account (20 digits): "));
            break;
        }
        catch (OrderException &e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    while (true) {
        try {
            order.set_recipient_account(ask_line("Recipient account (20 digits): "));
            break;
        }
        catch (OrderException &e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    while (true) {
        try {
            order.set_amount(ask_amount());
            break;
        }
        catch (OrderException &e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}

static void print_all(const OrderList &list) {
    if (list.get_count() == 0) {
        std::cout << "List is empty" << std::endl;
        return;
    }
    for (int i = 0; i < list.get_count(); i++) {
        std::cout << "#" << i + 1 << std::endl;
        std::cout << *list.get(i) << std::endl;
    }
}

// Добавление заказа на любую позицию
static void add_order(OrderList &list) {
    Order *order = new Order();
    try {
        read_order(*order);
        int position = 1; 
        if (list.get_count() > 0) {
            position = ask_int_in_range("Insert position (1.." +
                                        std::to_string(list.get_count() + 1) + "): ",
                                        1, list.get_count() + 1);
        }
        list.add_at(position - 1, order);
        order = NULL;
        list.sort_by_payer_account();
        std::cout << "Order added, the list is sorted by payer account" << std::endl;
    }
    catch (...) {
        delete order;
        throw;
    }
}

// Редактирование любой записи
static void edit_order(OrderList &list) {
    if (list.get_count() == 0) {
        std::cout << "List is empty" << std::endl;
        return;
    }
    print_all(list);
    int number = ask_int_in_range("Record number (0 - back): ", 0, list.get_count()); 
    if (number == 0) return;

    Order *copy = new Order(*list.get(number - 1)); 
    try {
        read_order(*copy);
        list.replace_at(number - 1, copy);
        copy = NULL;
        std::cout << "Order updated" << std::endl;
        list.sort_by_payer_account();
        std::cout << "List re-sorted by payer account" << std::endl;
    }
    catch (...) {
        delete copy;
        throw;
    }
}

// Удаление любой записи
static void remove_order(OrderList &list) {
    if (list.get_count() == 0) {
        std::cout << "List is empty" << std::endl;
        return;
    }
    print_all(list);
    int number = ask_int_in_range("Record number (0 - back): ", 0, list.get_count()); // Номер записи
    if (number == 0) return;
    list.remove_at(number - 1);
    std::cout << "Order removed" << std::endl;
}

// Сумма, снятая со счёта плательщика
static void find_amount(const OrderList &list) {
    std::string account = ask_line("Payer account (20 digits): "); // Искомый счёт
    if (!Order::is_valid_account(account)) {
        std::cout << "Error: invalid payer account" << std::endl;
        return;
    }
    int found = list.count_by_payer_account(account); 
    if (found == 0) {
        std::cout << "No orders with payer account " << account << std::endl;
        return;
    }
    std::cout << "Total amount withdrawn from payer account " << account << ": "
              << Order::format_amount(list.total_amount_by_payer_account(account))
              << " RUB (" << found << " order(s))" << std::endl;
}

// Сохранение списка в файл (путь вводится пользователем)
static void save_list(const OrderList &list) {
    std::string path = ask_line("File path to save: "); // Путь к файлу
    list.save_orders(path);
    std::cout << "File saved successfully: " << path << std::endl;
}

// Загрузка списка из файла (путь вводится пользователем)
static void load_list(OrderList &list) {
    std::string path = ask_line("File path to load: "); // Путь к файлу
    list.load_orders(path);
    std::cout << "Data loaded successfully: " << path << std::endl;
}

int main() {
    try {
        OrderList list; // Список заказов на всё время работы программы
        while (true) {
            std::cout << "\n=== ORDER LIST MENU ===\n\n"
                      << "1. Add order (at any position)\n"
                      << "2. Edit order\n"
                      << "3. Remove order\n"
                      << "4. Show all orders\n"
                      << "5. Find amount by payer account\n"
                      << "6. Sort by payer account\n"
                      << "7. Save to file\n"
                      << "8. Load from file\n"
                      << "0. Exit\n\n";
            int choice = ask_int_in_range("Select: ", 0, 8);
            if (choice == 0) break;
            try {
                switch (choice) {
                    case 1: add_order(list); break;
                    case 2: edit_order(list); break;
                    case 3: remove_order(list); break;
                    case 4: print_all(list); break;
                    case 5: find_amount(list); break;
                    case 6:
                        list.sort_by_payer_account();
                        std::cout << "List sorted" << std::endl; break;
                    case 7: save_list(list); break;
                    case 8: load_list(list); break;
                    default: break;
                }
            }
            catch (OrderException &e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        }
        std::cout << "Exit" << std::endl;
    }
    catch (std::exception &e) {
        std::cout << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
