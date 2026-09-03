#include "order.hpp"

#include <sstream>
#include <stdexcept>

Order::Order(
    const OrderId id,
    const Side side,
    const Price price,
    const Quantity quantity,
    const std::uint64_t sequence_number
)
    : id_(id),
      side_(side),
      price_(price),
      remaining_quantity_(quantity),
      sequence_number_(sequence_number)
{
    if (id == 0)
    {
        throw std::invalid_argument(
            "Order ID must be greater than zero"
        );
    }

    if (price <= 0)
    {
        throw std::invalid_argument(
            "Price must be greater than zero"
        );
    }

    if (quantity == 0)
    {
        throw std::invalid_argument(
            "Quantity must be greater than zero"
        );
    }
}

OrderId Order::id() const noexcept
{
    return id_;
}

Side Order::side() const noexcept
{
    return side_;
}

Price Order::price() const noexcept
{
    return price_;
}

Quantity Order::remaining_quantity() const noexcept
{
    return remaining_quantity_;
}

std::uint64_t Order::sequence_number() const noexcept
{
    return sequence_number_;
}

void Order::fill(const Quantity quantity)
{
    if (quantity == 0)
    {
        throw std::invalid_argument(
            "Fill quantity must be greater than zero"
        );
    }

    if (quantity > remaining_quantity_)
    {
        throw std::invalid_argument(
            "Fill quantity exceeds remaining order quantity"
        );
    }

    remaining_quantity_ -= quantity;
}

std::string Order::to_string() const
{
    std::ostringstream output;

    output << "Order{id=" << id_
           << ", side=" << side_to_string(side_)
           << ", price=" << price_
           << ", quantity=" << remaining_quantity_
           << ", sequence=" << sequence_number_
           << '}';

    return output.str();
}