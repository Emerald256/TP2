#include "order_exception.h"

OrderException::OrderException(std::string message) : std::runtime_error(message) {}
