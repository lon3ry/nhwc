#include <iostream>
#include <limits.h>

#include "util.hpp"

bool read_integer(long long& value)
{
    if (!(std::cin >> value))
        return false;
    // Reject tokens such as "12x"; reaching EOF after a number is valid.
    const auto next = std::cin.peek();
    return !std::cin.bad() && (next == std::char_traits<char>::eof() ||
           std::isspace(static_cast<unsigned char>(next)));
}

bool can_not_be_valid_size_t(long long value)
{
    return value < 0 ||
           static_cast<unsigned long long>(value) > std::numeric_limits<std::size_t>::max();
}
