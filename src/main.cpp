#include "order.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

bool is_valid_price(const std::int64_t price)
{
    return price > 0;
}

bool is_valid_quantity(const std::uint32_t quantity)
{
    return quantity > 0;
}

void print_validation_result(
    const std::string& name,
    const bool valid)
{
    std::cout << name << ": "
              << (valid ? "valid" : "invalid")
              << '\n';
}

int main()
{
    const std::int64_t price = 10'525;
    const std::uint32_t quantity = 50;

    print_validation_result(
        "Price",
        is_valid_price(price)
    );

    print_validation_result(
        "Quantity",
        is_valid_quantity(quantity)
    );

    try
    {
        const Order order(
            1001,
            Side::Buy,
            price,
            quantity,
            1
        );

        std::cout << order.to_string() << '\n';
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: "
                  << exception.what()
                  << '\n';

        return 1;
    }

    return 0;
}