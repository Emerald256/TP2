#pragma once
#include <iostream>
#include <string>

// Платёжное поручение: счёт плательщика, счёт получателя, перечисляемая сумма
class Order {
private:
    std::string payer_account;
    std::string recipient_account;
    double amount;
public:
    static constexpr int ACCOUNT_LENGTH = 20;     // Длина расчётного счёта в цифрах

    Order();
    Order(std::string new_payer_account, std::string new_recipient_account, double new_amount);
    Order(const Order &o);
    ~Order();

    void set_payer_account(std::string new_payer_account);
    void set_recipient_account(std::string new_recipient_account);
    void set_amount(double new_amount);

    std::string get_payer_account() const;
    std::string get_recipient_account() const;
    double get_amount() const;


    bool is_complete() const;                     // Все поля заполнены корректно?
    void check_complete() const;                  // Ошибка, если объект не заполнен


    static std::string format_amount(double value);// Сумма в виде строки с двумя знаками после точки
    static bool is_valid_account(const std::string &account);

    bool operator<(const Order &other) const;     // Сравнение для сортировки по счёту плательщика
    bool operator==(const Order &other) const;

    friend std::ostream& operator<<(std::ostream &os, const Order &o);
    friend std::istream& operator>>(std::istream &is, Order &o);
};
