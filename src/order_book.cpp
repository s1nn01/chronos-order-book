#include "order_book.hpp"

#include <iterator>
#include <cstdint>
#include <ostream>

bool OrderBook::add_order(const Order& order)
{
    if (contains(order.id()))
    {
        return false;
    }

    if (order.side() == Side::Buy)
    {
        auto& orders = bids_[order.price()];

        orders.push_back(order);

        const auto iterator =
            std::prev(orders.end());

        order_index_.emplace(
            order.id(),
            OrderLocation{
                Side::Buy,
                order.price(),
                iterator
            }
        );
    }
    else
    {
        auto& orders = asks_[order.price()];

        orders.push_back(order);

        const auto iterator =
            std::prev(orders.end());

        order_index_.emplace(
            order.id(),
            OrderLocation{
                Side::Sell,
                order.price(),
                iterator
            }
        );
    }

    return true;
}

bool OrderBook::cancel_order(const OrderId id)
{
    const auto index_iterator =
        order_index_.find(id);

    if (index_iterator == order_index_.end())
    {
        return false;
    }

    const OrderLocation location =
        index_iterator->second;

    if (location.side == Side::Buy)
    {
        auto price_level =
            bids_.find(location.price);

        if (price_level == bids_.end())
        {
            return false;
        }

        price_level->second.erase(
            location.iterator
        );

        if (price_level->second.empty())
        {
            bids_.erase(price_level);
        }
    }
    else
    {
        auto price_level =
            asks_.find(location.price);

        if (price_level == asks_.end())
        {
            return false;
        }

        price_level->second.erase(
            location.iterator
        );

        if (price_level->second.empty())
        {
            asks_.erase(price_level);
        }
    }

    order_index_.erase(index_iterator);

    return true;
}

bool OrderBook::contains(
    const OrderId id) const noexcept
{
    return order_index_.contains(id);
}

std::size_t OrderBook::size() const noexcept
{
    return order_index_.size();
}

std::optional<Price>
OrderBook::best_bid() const noexcept
{
    if (bids_.empty())
    {
        return std::nullopt;
    }

    return bids_.begin()->first;
}

std::optional<Price>
OrderBook::best_ask() const noexcept
{
    if (asks_.empty())
    {
        return std::nullopt;
    }

    return asks_.begin()->first;
}

void OrderBook::print_snapshot(
    std::ostream& output) const
{
    output << "\n========== ORDER BOOK ==========\n";

    output << "ASKS — lowest price first\n";

    if (asks_.empty())
    {
        output << "  <empty>\n";
    }
    else
    {
        for (const auto& [price, orders] : asks_)
        {
            std::uint64_t total_quantity = 0;

            for (const Order& order : orders)
            {
                total_quantity +=
                    order.remaining_quantity();
            }

            output << "  Price: " << price
                   << " | Total quantity: "
                   << total_quantity
                   << " | Orders: "
                   << orders.size()
                   << '\n';

            for (const Order& order : orders)
            {
                output
                    << "    ID: " << order.id()
                    << " | Quantity: "
                    << order.remaining_quantity()
                    << " | Sequence: "
                    << order.sequence_number()
                    << '\n';
            }
        }
    }

    output << "--------------------------------\n";

    output << "BIDS — highest price first\n";

    if (bids_.empty())
    {
        output << "  <empty>\n";
    }
    else
    {
        for (const auto& [price, orders] : bids_)
        {
            std::uint64_t total_quantity = 0;

            for (const Order& order : orders)
            {
                total_quantity +=
                    order.remaining_quantity();
            }

            output << "  Price: " << price
                   << " | Total quantity: "
                   << total_quantity
                   << " | Orders: "
                   << orders.size()
                   << '\n';

            for (const Order& order : orders)
            {
                output
                    << "    ID: " << order.id()
                    << " | Quantity: "
                    << order.remaining_quantity()
                    << " | Sequence: "
                    << order.sequence_number()
                    << '\n';
            }
        }
    }

    output << "================================\n\n";
}