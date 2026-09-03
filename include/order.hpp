#pragma once

#include "order_types.hpp"

#include <string>

class Order
{
public:
    Order(
        OrderId id,
        Side side,
        Price price,
        Quantity quantity,
        std::uint64_t sequence_number
    );

    [[nodiscard]] OrderId id() const noexcept;
    [[nodiscard]] Side side() const noexcept;
    [[nodiscard]] Price price() const noexcept;
    [[nodiscard]] Quantity remaining_quantity() const noexcept;
    [[nodiscard]] std::uint64_t sequence_number() const noexcept;

    void fill(Quantity quantity);

    [[nodiscard]] std::string to_string() const;

private:
    OrderId id_;
    Side side_;
    Price price_;
    Quantity remaining_quantity_;
    std::uint64_t sequence_number_;
};