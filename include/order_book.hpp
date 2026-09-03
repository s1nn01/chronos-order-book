#pragma once

#include "order.hpp"
#include "trade.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

class OrderBook
{
public:
    OrderBook() = default;

    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;

    bool add_order(const Order& order);
    bool cancel_order(OrderId id);

    // Matches an incoming limit order against the opposite side using
    // price-time priority. Any unfilled quantity rests on the book.
    [[nodiscard]] std::vector<Trade> submit_order(Order order);

    [[nodiscard]] bool contains(OrderId id) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] std::optional<Quantity>
    remaining_quantity(OrderId id) const noexcept;

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

    [[nodiscard]] std::vector<Trade>
    match_buy(Order& incoming);

    [[nodiscard]] std::vector<Trade>
    match_sell(Order& incoming);

    BidLevels bids_;
    AskLevels asks_;

    std::unordered_map<OrderId, OrderLocation>
        order_index_;

    std::uint64_t next_trade_sequence_{1};
};
