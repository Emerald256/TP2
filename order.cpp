#include "order.h"
#include "order_exception.h"
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <new>

// Максимальная сумма перевода (защита от переполнения при округлении)
static const double MAX_AMOUNT = 1000000000000.0;

// Проверка расчётного счёта: ровно 20 цифр
static void check_account(const std::string &account, const std::string &field_name) {
    if (account.size() != static_cast<size_t>(Order::ACCOUNT_LENGTH)) {
        throw OrderException(field_name + " must contain exactly " +
                             std::to_string(Order::ACCOUNT_LENGTH) + " digits");
    }
    for (size_t i = 0; i < account.size(); i++) {
        if (!std::isdigit(static_cast<unsigned char>(account[i]))) {
            throw OrderException(field_name + " must contain digits only");
        }
    }
}

// Преобразование строки в сумму с проверкой формата
static double parse_amount(const std::string &s) {
    size_t pos = 0;    // Позиция, на которой завершился разбор строки
    double value = 0;  // Полученная сумма
    try {
        value = std::stod(s, &pos);
    }
    catch (std::logic_error&) {
        throw OrderException("Order amount: \"" + s + "\" is not a valid number");
    }
    if (pos != s.size() || !std::isfinite(value)) {
        throw OrderException("Order amount: \"" + s + "\" is not a valid number");
    }
    return value;
}

Order::Order() {
    payer_account = "";
    recipient_account = "";
    amount = 0.0;
    std::cout << "Constructor for Order without params called (object "
              << static_cast<const void*>(this) << ")" << std::endl;
}

Order::Order(std::string new_payer_account, std::string new_recipient_account, double new_amount) {
    set_payer_account(new_payer_account);
    set_recipient_account(new_recipient_account);
    set_amount(new_amount);
    std::cout << "Constructor for Order (payer " << payer_account << ") with params called (object "
              << static_cast<const void*>(this) << ")" << std::endl;
}

Order::Order(const Order &o) {
    payer_account = o.payer_account;
    recipient_account = o.recipient_account;
    amount = o.amount;
    std::cout << "Copy constructor for Order (payer " << payer_account << ") called: source "
              << static_cast<const void*>(&o) << ", new object "
              << static_cast<const void*>(this) << std::endl;
}

Order::~Order() {
    std::cout << "Destructor for Order (payer " << payer_account << ") called (object "
              << static_cast<const void*>(this) << ")" << std::endl;
}

void Order::set_payer_account(std::string new_payer_account) {
    check_account(new_payer_account, "Payer account");
    payer_account = new_payer_account;
}

void Order::set_recipient_account(std::string new_recipient_account) {
    check_account(new_recipient_account, "Recipient account");
    recipient_account = new_recipient_account;
}

void Order::set_amount(double new_amount) {
    if (!std::isfinite(new_amount) || new_amount <= 0) {
        throw OrderException("Order amount must be greater than zero");
    }
    if (new_amount > MAX_AMOUNT) {
        throw OrderException("Order amount must not exceed 1000000000000 RUB");
    }
    double rounded = std::round(new_amount * 100.0) / 100.0;
    if (rounded <= 0) {
        throw OrderException("Order amount must be at least 0.01 RUB");
    }
    amount = rounded;
}

std::string Order::get_payer_account() const { return payer_account; }
std::string Order::get_recipient_account() const { return recipient_account; }
double Order::get_amount() const { return amount; }

bool Order::is_complete() const {
    return is_valid_account(payer_account) && is_valid_account(recipient_account) &&
           std::isfinite(amount) && amount > 0;
}

void Order::check_complete() const {
    if (!is_complete()) {
        throw OrderException("Order has empty or invalid fields");
    }
}

std::string Order::format_amount(double value) {
    try {
        std::ostringstream os; // Буфер для форматированной суммы
        os << std::fixed << std::setprecision(2) << value;
        return os.str();
    }
    catch (std::bad_alloc&) {
        throw OrderException("Not enough memory to format an amount");
    }
}

bool Order::is_valid_account(const std::string &account) {
    if (account.size() != static_cast<size_t>(ACCOUNT_LENGTH)) return false;
    for (size_t i = 0; i < account.size(); i++) {
        if (!std::isdigit(static_cast<unsigned char>(account[i]))) return false;
    }
    return true;
}

bool Order::operator<(const Order &other) const {
    if (payer_account != other.payer_account) return payer_account < other.payer_account;
    if (recipient_account != other.recipient_account) return recipient_account < other.recipient_account;
    return amount < other.amount;
}

bool Order::operator==(const Order &other) const {
    return payer_account == other.payer_account &&
           recipient_account == other.recipient_account &&
           amount == other.amount;
}

std::ostream& operator<<(std::ostream &os, const Order &o) {
    os << "Payer account: " << o.payer_account << "\n"
       << "Recipient account: " << o.recipient_account << "\n"
       << "Amount: " << Order::format_amount(o.amount) << " RUB";
    return os;
}

std::istream& operator>>(std::istream &is, Order &o) {
    std::string payer, recipient, amount_text; // Буферы для трёх полей записи
    if (!(is >> payer)) throw OrderException("Failed to read payer account");
    if (!(is >> recipient)) throw OrderException("Failed to read recipient account");
    if (!(is >> amount_text)) throw OrderException("Failed to read amount");

    o.set_payer_account(payer);
    o.set_recipient_account(recipient);
    o.set_amount(parse_amount(amount_text));
    return is;
}
