#pragma once

#include "order_types.hpp"

enum class CommandType
{
    Add,
    Cancel,
    Exit
};

struct Command
{
    CommandType type;
    OrderId id{0};
    Side side{Side::Buy};
    Price price{0};
    Quantity quantity{0};
};