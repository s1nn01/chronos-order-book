#pragma once

#include "order.hpp"

#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <iosfwd>

class OrderBook
{
public:
    OrderBook() = default;

    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;

    bool add_order(const Order& order);
    bool cancel_order(OrderId id);

    [[nodiscard]] bool contains(OrderId id) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] std::optional<Price>
    best_bid() const noexcept;

    [[nodiscard]] std::optional<Price>
    best_ask() const noexcept;

    void print_snapshot(std::ostream& output) const;

private:
    using OrderQueue = std::list<Order>;

    using BidLevels = std::map<
        Price,
        OrderQueue,
        std::greater<Price>
    >;

    using AskLevels = std::map<
        Price,
        OrderQueue,
        std::less<Price>
    >;

    struct OrderLocation
    {
        Side side;
        Price price;
        OrderQueue::iterator iterator;
    };

    BidLevels bids_;
    AskLevels asks_;

    std::unordered_map<OrderId, OrderLocation>
        order_index_;
};