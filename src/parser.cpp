#include "parser.hpp"

#include <limits>
#include <sstream>
#include <string>

namespace
{
    std::optional<Side> parse_side(
        const std::string& text)
    {
        if (text == "BUY")
        {
            return Side::Buy;
        }

        if (text == "SELL")
        {
            return Side::Sell;
        }

        return std::nullopt;
    }

    bool has_extra_input(std::istringstream& input)
    {
        std::string extra;
        return static_cast<bool>(input >> extra);
    }
}

std::optional<Command> parse_command(
    const std::string& line,
    std::string& error)
{
    std::istringstream input(line);

    std::string command_name;

    if (!(input >> command_name))
    {
        error = "Command cannot be empty";
        return std::nullopt;
    }

    if (command_name == "EXIT")
    {
        if (has_extra_input(input))
        {
            error = "EXIT does not accept arguments";
            return std::nullopt;
        }

        return Command{
            .type = CommandType::Exit
        };
    }

    if (command_name == "CANCEL")
    {
        unsigned long long raw_id = 0;

        if (!(input >> raw_id))
        {
            error = "Expected: CANCEL <order_id>";
            return std::nullopt;
        }

        if (raw_id == 0)
        {
            error = "Order ID must be greater than zero";
            return std::nullopt;
        }

        if (has_extra_input(input))
        {
            error = "Too many values for CANCEL";
            return std::nullopt;
        }

        return Command{
            .type = CommandType::Cancel,
            .id = static_cast<OrderId>(raw_id)
        };
    }

    if (command_name == "ADD")
    {
        unsigned long long raw_id = 0;
        std::string side_text;
        long long raw_price = 0;
        long long raw_quantity = 0;

        if (!(input
              >> raw_id
              >> side_text
              >> raw_price
              >> raw_quantity))
        {
            error =
                "Expected: ADD <id> <BUY|SELL> "
                "<price> <quantity>";

            return std::nullopt;
        }

        const auto side = parse_side(side_text);

        if (!side.has_value())
        {
            error = "Side must be BUY or SELL";
            return std::nullopt;
        }

        if (raw_id == 0)
        {
            error = "Order ID must be greater than zero";
            return std::nullopt;
        }

        if (raw_price <= 0)
        {
            error = "Price must be greater than zero";
            return std::nullopt;
        }

        if (
            raw_quantity <= 0 ||
            raw_quantity >
                std::numeric_limits<Quantity>::max()
        )
        {
            error = "Quantity is outside the valid range";
            return std::nullopt;
        }

        if (has_extra_input(input))
        {
            error = "Too many values for ADD";
            return std::nullopt;
        }

        return Command{
            .type = CommandType::Add,
            .id = static_cast<OrderId>(raw_id),
            .side = side.value(),
            .price = static_cast<Price>(raw_price),
            .quantity =
                static_cast<Quantity>(raw_quantity)
        };
    }

    error = "Unknown command: " + command_name;
    return std::nullopt;
}