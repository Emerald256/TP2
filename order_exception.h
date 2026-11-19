#pragma once
#include <stdexcept>
#include <string>

// Исключение программы: message поясняет, что именно произошло
class OrderException : public std::runtime_error {
public:
    OrderException(std::string message);
};
