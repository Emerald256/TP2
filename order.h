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
    static constexpr char FIELD_SEP = ';';        // Разделитель полей в файле

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

    // Универсальный доступ к полям (меню)
    int field_count() const;
    std::string field_name(int index) const;
    std::string get_field(int index) const;
    void set_field(int index, std::string value);
    bool field_is_numeric(int index) const;

    bool is_complete() const;                     // Все поля заполнены корректно?
    void check_complete() const;                  // Ошибка, если объект не заполнен

    std::string to_file_line() const;             // Упаковка полей в строку файла
    void load_from_file_line(std::string line);   // Загрузка полей из строки файла

    static std::string format_amount(double value);// Сумма в виде строки с двумя знаками после точки
    static bool is_valid_account(const std::string &account);

    bool operator<(const Order &other) const;     // Сравнение для сортировки по счёту плательщика
    bool operator==(const Order &other) const;

    friend std::ostream& operator<<(std::ostream &os, const Order &o);
    friend std::istream& operator>>(std::istream &is, Order &o);
};
