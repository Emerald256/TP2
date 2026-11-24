#include "menu.h"
#include "order_exception.h"
#include <iostream>
#include <new>

// Проверка на пустую строку
static bool is_blank(const std::string &s) {
    return s.find_first_not_of(" \t\r\n") == std::string::npos;
}

// Убирает пробелы и переводы строк по краям
static std::string trim_copy(const std::string &s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

// Проверка допустимости имени файла
static void check_file_name(const std::string &name) {
    if (is_blank(name)) {
        throw OrderException("File name must not be empty");
    }
    const std::string forbidden = "<>:\"/\\|?*"; // Символы, недопустимые в имени файла
    if (name.find_first_of(forbidden) != std::string::npos) {
        throw OrderException("File name must not contain path separators or characters < > : \" / \\ | ? *");
    }
    char last = name[name.size() - 1];
    if (last == '.') {
        throw OrderException("File name must not end with a dot");
    }
}

// Проверка пути к папке
static void check_folder_path(const std::string &folder) {
    std::string trimmed = trim_copy(folder);
    if (is_blank(trimmed)) {
        throw OrderException("Folder path must not be empty");
    }
}

// Объединение пути к папке и имени файла
static std::string join_path(const std::string &folder, const std::string &file) {
    char last = folder[folder.size() - 1]; // Проверяем, есть ли уже разделитель в конце
    if (last == '/' || last == '\\') return folder + file;
    return folder + "\\" + file;
}

// Правила ввода имени файла
static void print_file_name_rules() {
    std::cout << "File name rules:" << std::endl;
    std::cout << "- enter only the name, without extension;" << std::endl;
    std::cout << "- the program adds .txt automatically;" << std::endl;
    std::cout << "- path separators and characters < > : \" / \\ | ? * are not allowed;" << std::endl;
    std::cout << "- if you enter an extension, it becomes part of the name (the program still adds .txt)" << std::endl;
}

Menu::Menu(OrderList &l) : list(l), order_initialized(false) {
    std::cout << "Constructor for Menu called" << std::endl;
}

Menu::~Menu() {
    std::cout << "Destructor for Menu called" << std::endl;
}

std::string Menu::ask_line(std::string prompt) {
    std::cout << prompt;
    std::string s;
    if (!std::getline(std::cin, s)) {
        throw std::runtime_error("Input stream closed");
    }
    return s;
}

int Menu::parse_int(std::string s) {
    size_t pos = 0;
    int value = 0;
    try {
        value = std::stoi(s, &pos);
    }
    catch (std::logic_error&) {
        throw OrderException("\"" + s + "\" is not a valid integer");
    }
    if (pos != s.size()) {
        throw OrderException("\"" + s + "\" is not a valid integer");
    }
    return value;
}

int Menu::ask_int(std::string prompt) {
    while (true) {
        try {
            return parse_int(ask_line(prompt));
        }
        catch (OrderException &e) {
            std::cout << "Error: " << e.what() << " Try again" << std::endl;
        }
    }
}

int Menu::ask_int_in_range(std::string prompt, int min_value, int max_value) {
    while (true) {
        int value = ask_int(prompt);
        if (value >= min_value && value <= max_value) return value;
        std::cout << "Incorrect value, enter a number from " << min_value
                  << " to " << max_value << std::endl;
    }
}

void Menu::read_order_fields(Order &order) {
    for (int i = 0; i < order.field_count(); i++) {
        while (true) {
            std::string value = ask_line("Input " + order.field_name(i) + ": ");
            try {
                order.set_field(i, value);
                break;
            }
            catch (OrderException &e) {
                std::cout << "Error: " << e.what() << " Try again" << std::endl;
            }
        }
    }
}

void Menu::print_all() {
    if (list.get_count() == 0) {
        std::cout << "List is empty" << std::endl;
        return;
    }
    for (int i = 0; i < list.get_count(); i++) {
        std::cout << "#" << i + 1 << std::endl;
        std::cout << *list.get(i) << std::endl;
    }
}

int Menu::ask_record_index(std::string action) {
    if (list.get_count() == 0) {
        std::cout << "List is empty" << std::endl;
        return -1;
    }
    std::cout << action << ":" << std::endl;
    print_all();
    int number = ask_int_in_range("Record number (0 - back): ", 0, list.get_count());
    if (number == 0) return -1;
    return number - 1;
}

bool Menu::ask_filepath(std::string &filepath, std::string action) {
    std::cout << action << ":" << std::endl;
    std::cout << "1. File in current folder" << std::endl;
    std::cout << "2. Full path" << std::endl;
    std::cout << "0. Back" << std::endl << std::endl;

    int mode = ask_int_in_range("Select: ", 0, 2); // Способ выбора файла
    if (mode == 0) return false;
    if (mode == 1) {
        print_file_name_rules();
        std::string name = ask_line("File name: "); // Имя файла без расширения
        name = trim_copy(name);
        check_file_name(name);
        filepath = name + ".txt";
        return true;
    }

    std::string folder = ask_line("Folder path: ");
    check_folder_path(folder);
    print_file_name_rules();
    std::string name = ask_line("File name: "); // Имя файла без расширения
    name = trim_copy(name);
    check_file_name(name);
    folder = trim_copy(folder);
    filepath = join_path(folder, name + ".txt");
    return true;
}

bool Menu::startup_dialog() {
    while (true) {
        std::cout << "=== ORDER LIST ===\nSelect initialization option:\n\n"
                  << "1 - Create empty list\n"
                  << "2 - Load data from file\n"
                  << "0 - Exit\n\n";
        int selection = ask_int_in_range("Select: ", 0, 2); // Выбранный способ инициализации
        if (selection == 0) return false;
        if (selection == 1) return true;
        if (selection == 2) {
            if (handle_load_orders()) return true;
            std::cout << "Returning to initialization menu\n" << std::endl;
        }
    }
}

int Menu::ask_order_choice() {
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
    return ask_int_in_range("Select: ", 0, 8);
}

void Menu::run() {
    run_order_mode();
}

void Menu::run_order_mode() {
    if (!order_initialized) {
        if (!startup_dialog()) return;
        order_initialized = true;
    }

    while (true) {
        int choice = ask_order_choice(); // Выбранный пункт меню заказов
        if (choice == 0) return;
        try {
            switch (choice) {
                case 1: handle_add_order(); break;
                case 2: handle_edit_order(); break;
                case 3: handle_remove_order(); break;
                case 4: print_all(); break;
                case 5: handle_search_orders_by_payer(); break;
                case 6: handle_sort_orders(); break;
                case 7: handle_save_orders(); break;
                case 8: handle_load_orders(); break;
                default: break;
            }
        }
        catch (OrderException &e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}

void Menu::handle_add_order() {
    Order *order = NULL;
    try {
        order = new Order();
        std::cout << "Input new order data" << std::endl;
        read_order_fields(*order);

        int position = 1; // Позиция вставки; по умолчанию в конец
        if (list.get_count() > 0) {
            position = ask_int_in_range("Insert position (1.." +
                                        std::to_string(list.get_count() + 1) + "): ",
                                        1, list.get_count() + 1);
        }

        list.add_at(position - 1, order);
        order = NULL;
        list.sort_by_payer_account();
        std::cout << "Order inserted at requested position " << position
                  << " and the list was re-sorted by payer account" << std::endl;
    }
    catch (std::bad_alloc&) {
        delete order;
        throw OrderException("Not enough memory to add an order");
    }
    catch (...) {
        delete order;
        throw;
    }
}

void Menu::handle_edit_order() {
    int index = ask_record_index("Edit order");
    if (index < 0) return;

    Order *copy = NULL; // Копия записи: изменения применяются только после подтверждения
    try {
        copy = new Order(*list.get(index));
        std::cout << "Input new values, all fields are required" << std::endl;
        read_order_fields(*copy);

        std::cout << "New values:" << std::endl;
        std::cout << *copy << std::endl;
        std::cout << "\n1. Save changes\n0. Discard changes\n";
        int choice = ask_int_in_range("Select: ", 0, 1); // Подтверждение изменений
        if (choice == 0) {
            delete copy;
            std::cout << "Changes discarded" << std::endl;
            return;
        }

        list.replace_at(index, copy);
        copy = NULL;
        std::cout << "Order updated" << std::endl;
        std::cout << *list.get(index) << std::endl;
        list.sort_by_payer_account();
        std::cout << "List re-sorted by payer account" << std::endl;
    }
    catch (std::bad_alloc&) {
        delete copy;
        throw OrderException("Not enough memory to edit an order");
    }
    catch (...) {
        delete copy;
        throw;
    }
}

void Menu::handle_remove_order() {
    int index = ask_record_index("Remove order"); // Индекс удаляемой записи
    if (index < 0) return;

    std::cout << *list.get(index) << std::endl;
    std::cout << "\n1. Remove\n0. Cancel\n";
    int choice = ask_int_in_range("Select: ", 0, 1); // Подтверждение удаления
    if (choice == 0) {
        std::cout << "Removal cancelled" << std::endl;
        return;
    }

    list.remove_at(index);
    std::cout << "Order removed" << std::endl;
}

void Menu::handle_search_orders_by_payer() {
    std::cout << "Payer account must consist of exactly "
              << Order::ACCOUNT_LENGTH << " digits" << std::endl;
    std::string account = trim_copy(ask_line("Payer account: ")); // Искомый счёт плательщика
    if (!Order::is_valid_account(account)) {
        std::cout << "Error: invalid payer account" << std::endl;
        return;
    }

    int found = list.count_by_payer_account(account); // Количество найденных заказов
    if (found == 0) {
        std::cout << "No orders with payer account " << account << std::endl;
        return;
    }

    std::cout << "Orders with payer account " << account << ":" << std::endl;
    for (int i = 0; i < list.get_count(); i++) {
        if (list.get(i)->get_payer_account() == account) {
            std::cout << "#" << i + 1 << std::endl;
            std::cout << *list.get(i) << std::endl;
        }
    }

    double total = list.total_amount_by_payer_account(account); // Сумма по этому счёту
    std::cout << "Total amount withdrawn from payer account " << account
              << ": " << Order::format_amount(total) << " RUB ("
              << found << " order(s))" << std::endl;
}

void Menu::handle_sort_orders() {
    if (list.get_count() == 0) {
        std::cout << "List is empty" << std::endl;
        return;
    }
    list.sort_by_payer_account();
    std::cout << "List sorted by payer account" << std::endl;
}

void Menu::handle_save_orders() {
    try {
        std::string filepath;
        if (!ask_filepath(filepath, "Save to file")) return;
        list.save_orders(filepath);
        std::cout << "File saved successfully: " << filepath << std::endl;
    }
    catch (OrderException &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    catch (std::bad_alloc&) {
        std::cout << "Error: not enough memory" << std::endl;
    }
}

bool Menu::handle_load_orders() {
    try {
        std::string filepath;
        if (!ask_filepath(filepath, "Load from file")) return false;
        list.load_orders(filepath);
        std::cout << "Data loaded successfully: " << filepath << std::endl;
        return true;
    }
    catch (OrderException &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    catch (std::bad_alloc&) {
        std::cout << "Error: not enough memory" << std::endl;
    }
    return false;
}
