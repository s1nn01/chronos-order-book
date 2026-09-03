#pragma once

#include "order_types.hpp"

#include <cstdint>

struct Trade
{
    std::uint64_t sequence_number;
    OrderId maker_order_id;
    OrderId taker_order_id;
    Price price;
    Quantity quantity;
};
