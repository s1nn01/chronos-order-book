#pragma once

#include <cstdint>
#include <string_view>

enum class Side
{
    Buy,
    Sell
};

using OrderId = std::uint64_t;
using Price = std::int64_t;
using Quantity = std::uint32_t;

inline std::string_view side_to_string(const Side side)
{
    switch (side)
    {
        case Side::Buy:
            return "BUY";

        case Side::Sell:
            return "SELL";
    }

    return "UNKNOWN";
}