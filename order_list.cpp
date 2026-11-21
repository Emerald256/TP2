#include "order_list.h"
#include "order_exception.h"
#include <iostream>
#include <new>

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

OrderList& OrderList::operator+=(Order *o) {
    add(o);
    return *this;
}
