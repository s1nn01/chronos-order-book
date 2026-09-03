#include "order.hpp"
#include "order_book.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>

namespace
{
    class Lcg
    {
    public:
        explicit Lcg(std::uint64_t seed)
            : state_(seed)
        {
        }

        std::uint32_t next()
        {
            state_ = state_ * 6364136223846793005ULL + 1ULL;
            return static_cast<std::uint32_t>(state_ >> 32U);
        }

    private:
        std::uint64_t state_;
    };
}

int main()
{
    constexpr std::size_t event_count = 1'000'000;

    OrderBook book;
    Lcg rng{42};

    std::uint64_t trade_count = 0;
    std::uint64_t matched_quantity = 0;

    const auto start = std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < event_count; ++i)
    {
        const Side side =
            (i % 2 == 0) ? Side::Buy : Side::Sell;

        const Price jitter =
            static_cast<Price>(rng.next() % 101) - 50;

        const Price price =
            side == Side::Buy
                ? 10'000 + jitter
                : 10'000 - jitter;

        const Quantity quantity =
            static_cast<Quantity>(1 + (rng.next() % 100));

        const Order order(
            static_cast<OrderId>(i + 1),
            side,
            price,
            quantity,
            static_cast<std::uint64_t>(i + 1)
        );

        const auto trades = book.submit_order(order);

        trade_count += trades.size();

        for (const auto& trade : trades)
        {
            matched_quantity += trade.quantity;
        }
    }

    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = end - start;

    const double events_per_second =
        static_cast<double>(event_count) / elapsed.count();

    std::cout << "Chronos synthetic benchmark\n";
    std::cout << "Events processed: " << event_count << '\n';
    std::cout << "Trades generated: " << trade_count << '\n';
    std::cout << "Matched quantity: " << matched_quantity << '\n';
    std::cout << "Resting orders: " << book.size() << '\n';
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Elapsed seconds: " << elapsed.count() << '\n';
    std::cout << "Throughput: " << events_per_second
              << " events/second\n";

    return 0;
}
