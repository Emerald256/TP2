#pragma once
#include <string>
#include "order.h"

// Динамический массив платёжных поручений
// Хранит объекты Order и поддерживает их упорядочивание по счёту плательщика
class OrderList {
private:
    Order **items;
    int count;

    void swap_with(OrderList &other);            // Обмен содержимым
    void clear();                                // Удаляет все объекты и массив
public:
    OrderList();
    OrderList(const OrderList &other);
    OrderList& operator=(const OrderList &other);
    ~OrderList();

    void add(Order *o);                          // Добавляет объект в конец
    void add_at(int index, Order *o);            // Добавляет объект на позицию index
    void remove_at(int index);                   // Удаляет объект по индексу
    void replace_at(int index, Order *o);        // Заменяет объект по индексу
    Order* get(int index) const;                 // Объект по индексу
    int get_count() const;

    void sort_by_payer_account();                // Сортировка по счёту плательщика
    int count_by_payer_account(const std::string &payer) const;
    double total_amount_by_payer_account(const std::string &payer) const;


    OrderList& operator+=(Order *o);             // Короткая запись add
};
