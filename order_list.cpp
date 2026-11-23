#include "order_list.h"
#include "order_exception.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <new>
#include <sstream>

// Проверка на корректность пути к файлу
static void check_filepath(const std::string &filepath) {
    if (filepath.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw OrderException("File path must not be empty");
    }
}

// Чтение следующей строки из строкового потока
static std::string next_line(std::istringstream &stream) {
    std::string line;
    if (!std::getline(stream, line)) {
        throw OrderException("Invalid file format: unexpected end of file");
    }
    if (!line.empty() && line[line.size() - 1] == '\r') {
        line.erase(line.size() - 1);
    }
    return line;
}

// Преобразование строки в количество объектов
static int parse_count(std::string s) {
    size_t pos = 0;   // Позиция, на которой завершился разбор
    int value = 0;    // Полученное количество записей
    try {
        value = std::stoi(s, &pos);
    }
    catch (std::logic_error&) {
        throw OrderException("Invalid file format: wrong number of orders \"" + s + "\"");
    }
    if (pos != s.size()) {
        throw OrderException("Invalid file format: wrong number of orders \"" + s + "\"");
    }
    if (value < 0) {
        throw OrderException("Invalid file format: negative number of orders");
    }
    return value;
}

OrderList::OrderList() {
    items = NULL;
    count = 0;
    std::cout << "Constructor for OrderList without params called" << std::endl;
}

OrderList::OrderList(const OrderList &other) {
    items = NULL;
    count = 0;
    try {
        for (int i = 0; i < other.count; i++) {
            Order *copy = new Order(*other.items[i]);
            try {
                add(copy);
            }
            catch (...) {
                delete copy;
                throw;
            }
        }
    }
    catch (...) {
        clear();
        throw;
    }
    std::cout << "Copy constructor for OrderList called" << std::endl;
}

OrderList& OrderList::operator=(const OrderList &other) {
    if (this == &other) return *this;
    OrderList tmp(other);
    swap_with(tmp);
    std::cout << "Assignment operator for OrderList called" << std::endl;
    return *this;
}

OrderList::~OrderList() {
    clear();
    std::cout << "Destructor for OrderList called" << std::endl;
}

void OrderList::swap_with(OrderList &other) {
    Order **tmp_items = items;
    items = other.items;
    other.items = tmp_items;

    int tmp_count = count;
    count = other.count;
    other.count = tmp_count;
}

void OrderList::clear() {
    for (int i = 0; i < count; i++) {
        delete items[i];
    }
    delete[] items;
    items = NULL;
    count = 0;
}

void OrderList::add(Order *o) {
    add_at(count, o);
}

void OrderList::add_at(int index, Order *o) {
    if (index < 0 || index > count) {
        throw OrderException("Insert position is out of range");
    }
    if (o == NULL) {
        throw OrderException("Cannot add a null order");
    }
    o->check_complete();

    Order **new_items = NULL;
    try {
        new_items = new Order*[count + 1];
    }
    catch (std::bad_alloc&) {
        throw OrderException("Not enough memory to add an order");
    }

    for (int i = 0; i < index; i++) {
        new_items[i] = items[i];
    }
    new_items[index] = o;
    for (int i = index; i < count; i++) {
        new_items[i + 1] = items[i];
    }

    delete[] items;
    items = new_items;
    count++;
}

void OrderList::remove_at(int index) {
    if (index < 0 || index >= count) {
        throw OrderException("Order index is out of range");
    }

    if (count == 1) {
        delete items[index];
        delete[] items;
        items = NULL;
        count = 0;
        return;
    }

    Order **new_items = NULL;
    try {
        new_items = new Order*[count - 1];
    }
    catch (std::bad_alloc&) {
        throw OrderException("Not enough memory to remove an order");
    }

    int j = 0;
    for (int i = 0; i < count; i++) {
        if (i != index) {
            new_items[j] = items[i];
            j++;
        }
    }

    delete items[index];
    delete[] items;
    items = new_items;
    count--;
}

void OrderList::replace_at(int index, Order *o) {
    if (index < 0 || index >= count) {
        throw OrderException("Order index is out of range");
    }
    if (o == NULL) {
        throw OrderException("Cannot replace with a null order");
    }
    o->check_complete();
    if (items[index] == o) return;

    for (int i = 0; i < count; i++) {
        if (items[i] == o) {
            throw OrderException("Order is already in this list");
        }
    }

    Order *old = items[index]; // Сохраняем старый объект, чтобы удалить после замены
    items[index] = o;
    delete old;
}

Order* OrderList::get(int index) const {
    if (index < 0 || index >= count) {
        throw OrderException("Order index is out of range");
    }
    return items[index];
}

int OrderList::get_count() const { return count; }

void OrderList::sort_by_payer_account() {
    // Сортировка вставками
    for (int i = 1; i < count; i++) {
        Order *key = items[i];
        int j = i - 1;
        while (j >= 0 && *key < *items[j]) {
            items[j + 1] = items[j];
            j--;
        }
        items[j + 1] = key;
    }
}

int OrderList::count_by_payer_account(const std::string &payer) const {
    int result = 0;
    for (int i = 0; i < count; i++) {
        if (items[i]->get_payer_account() == payer) result++;
    }
    return result;
}

double OrderList::total_amount_by_payer_account(const std::string &payer) const {
    double total = 0; // Сумма по выбранному счёту плательщика
    for (int i = 0; i < count; i++) {
        if (items[i]->get_payer_account() == payer) {
            total += items[i]->get_amount();
        }
    }
    return total;
}

// Формат файла:
//   ORDER_LIST
//   <количество записей>
//   <счёт плательщика>;<счёт получателя>;<сумма>
//   ...
//   END
std::string OrderList::all_to_string() const {
    try {
        std::string result = "ORDER_LIST\n";
        result += std::to_string(count) + "\n";
        for (int i = 0; i < count; i++) {
            result += items[i]->to_file_line() + "\n";
        }
        result += "END\n";
        return result;
    }
    catch (std::bad_alloc&) {
        throw OrderException("Not enough memory to save orders");
    }
}

void OrderList::load_all_from_string(std::string text) {
    try {
        std::istringstream stream(text); // Поток для построчного разбора файла
        if (next_line(stream) != "ORDER_LIST") {
            throw OrderException("Invalid file format: no starting tag ORDER_LIST");
        }

        int n = parse_count(next_line(stream)); 
        OrderList tmp;
        for (int i = 0; i < n; i++) {
            std::string line = next_line(stream);
            Order *obj = new Order();           
            try {
                obj->load_from_file_line(line);
                tmp.add(obj);
            }
            catch (OrderException &e) {
                delete obj;
                throw OrderException("Invalid order record #" + std::to_string(i + 1) + ": " + e.what());
            }
            catch (...) {
                delete obj;
                throw;
            }
        }

        if (next_line(stream) != "END") {
            throw OrderException("Invalid file format: no ending tag END");
        }

        std::string extra; // Строка после END для проверки лишних данных
        while (std::getline(stream, extra)) {
            if (extra.find_first_not_of(" \t\r") != std::string::npos) {
                throw OrderException("Invalid file format: extra data after END");
            }
        }

        tmp.sort_by_payer_account();
        swap_with(tmp);
    }
    catch (std::bad_alloc&) {
        throw OrderException("Not enough memory to load orders");
    }
}

void OrderList::load_orders(std::string filepath) {
    check_filepath(filepath);
    try {
        std::ifstream file(filepath.c_str()); // Поток чтения файла
        if (!file.is_open()) {
            throw OrderException("Failed to open file \"" + filepath + "\" for reading");
        }

        std::string data, line; // Содержимое файла (data) и очередная строка (line)
        while (std::getline(file, line)) {
            data += line;
            data += "\n";
        }
        if (file.bad()) {
            throw OrderException("Failed to read file \"" + filepath + "\"");
        }
        load_all_from_string(data);
    }
    catch (std::bad_alloc&) {
        throw OrderException("Not enough memory to read file \"" + filepath + "\"");
    }
}

void OrderList::save_orders(std::string filepath) const {
    check_filepath(filepath);

    std::string tmp_path, bak_path; // Имена временного файла и резервной копии
    bool target_moved = false;      // Старый файл уже перемещён в .bak
    try {
        tmp_path = filepath + ".tmp";
        bak_path = filepath + ".bak";
        std::string data = all_to_string();

        std::ofstream file(tmp_path.c_str()); // Поток записи во временный файл
        if (!file.is_open()) {
            throw OrderException("Failed to open file \"" + tmp_path + "\" for writing");
        }
        file << data;
        file.close();
        if (file.fail()) {
            std::remove(tmp_path.c_str());
            throw OrderException("Write error: file \"" + filepath + "\" was not saved");
        }

        std::ifstream check(filepath.c_str()); // Проверяем, существует ли основной файл
        bool target_exists = check.good();     // Нужно ли делать резервную копию
        check.close();

        if (target_exists) {
            std::remove(bak_path.c_str());
            if (std::rename(filepath.c_str(), bak_path.c_str()) != 0) {
                std::remove(tmp_path.c_str());
                throw OrderException("Failed to create backup file \"" + bak_path + "\"");
            }
            target_moved = true;
        }

        if (std::rename(tmp_path.c_str(), filepath.c_str()) != 0) {
            if (target_moved && std::rename(bak_path.c_str(), filepath.c_str()) == 0) {
                target_moved = false;
            }
            std::remove(tmp_path.c_str());
            throw OrderException("Failed to replace file \"" + filepath + "\"");
        }
        target_moved = false;
    }
    catch (std::bad_alloc&) {
        if (target_moved) {
            std::rename(bak_path.c_str(), filepath.c_str());
        }
        std::remove(tmp_path.c_str());
        throw OrderException("Not enough memory to save orders");
    }
    catch (...) {
        if (target_moved) {
            std::rename(bak_path.c_str(), filepath.c_str());
        }
        std::remove(tmp_path.c_str());
        throw;
    }
}

OrderList& OrderList::operator+=(Order *o) {
    add(o);
    return *this;
}
