#include <iostream>
#include <sstream>
#include <string>
#include "order.h"
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
static Order* read_order_by_fields() {
    Order *order = new Order();
    try {
        while (true) {
            try {
                order->set_payer_account(ask_line("Payer account (20 digits): "));
                break;
            }
            catch (OrderException &e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        }
        while (true) {
            try {
                order->set_recipient_account(ask_line("Recipient account (20 digits): "));
                break;
            }
            catch (OrderException &e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        }
        while (true) {
            try {
                order->set_amount(ask_amount());
                break;
            }
            catch (OrderException &e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        }
    }
    catch (...) {
        delete order;
        throw;
    }
    return order;
}

// Ввод заказа одной строкой через перегруженный operator>>
static Order* read_order_in_line() {
    Order *order = new Order();
    try {
        std::string line = ask_line("Payer account, recipient account and amount: ");
        std::istringstream is(line);
        is >> *order;
    }
    catch (...) {
        delete order;
        throw;
    }
    return order;
}

static void change_field(Order &order) {
    std::cout << "1. Payer account\n2. Recipient account\n3. Amount\n0. Back\n";
    int field = ask_int_in_range("Field: ", 0, 3);
    if (field == 0) return;
    if (field == 1) order.set_payer_account(ask_line("New payer account: "));
    else if (field == 2) order.set_recipient_account(ask_line("New recipient account: "));
    else order.set_amount(ask_amount());
    std::cout << "Field changed" << std::endl;
}

int main() {
    Order *order = NULL;
    try {
        while (true) {
            std::cout << "\n=== ORDER MENU ===\n\n"
                      << "1. Input order by fields\n"
                      << "2. Input order in one line\n"
                      << "3. Show order\n"
                      << "4. Change one field\n"
                      << "0. Exit\n\n";
            int choice = ask_int_in_range("Select: ", 0, 5);
            if (choice == 0) break;

            try {
                if (choice == 1 || choice == 2) {
                    Order *new_order = (choice == 1) ? read_order_by_fields() : read_order_in_line();
                    delete order;
                    order = new_order;
                    std::cout << "Order created" << std::endl;
                    continue;
                }
                if (order == NULL) {
                    std::cout << "No order yet, create it first" << std::endl;
                    continue;
                }
                if (choice == 3) {
                    std::cout << *order << std::endl;
                }
                else if (choice == 4) {
                    change_field(*order);
                }
            }
            catch (OrderException &e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        }
    }
    catch (std::exception &e) {
        delete order;
        std::cout << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    delete order;
    std::cout << "Exit" << std::endl;
    return 0;
}
