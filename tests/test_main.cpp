#include "order.hpp"
#include "order_book.hpp"
#include "parser.hpp"

#include <exception>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    struct TestFailure : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    void check(bool condition, const std::string& message)
    {
        if (!condition)
        {
            throw TestFailure(message);
        }
    }

    template <typename T, typename U>
    void check_equal(
        const T& actual,
        const U& expected,
        const std::string& message)
    {
        if (!(actual == expected))
        {
            throw TestFailure(message);
        }
    }

    template <typename Fn>
    void check_throws(Fn&& fn, const std::string& message)
    {
        bool threw = false;

        try
        {
            fn();
        }
        catch (const std::exception&)
        {
            threw = true;
        }

        check(threw, message);
    }

    Order buy(
        OrderId id,
        Price price,
        Quantity quantity,
        std::uint64_t sequence = 1)
    {
        return Order{id, Side::Buy, price, quantity, sequence};
    }

    Order sell(
        OrderId id,
        Price price,
        Quantity quantity,
        std::uint64_t sequence = 1)
    {
        return Order{id, Side::Sell, price, quantity, sequence};
    }

    using Test = std::pair<std::string, std::function<void()>>;
}

int main()
{
    const std::vector<Test> tests = {
        {"order rejects zero id", [] {
            check_throws([] { Order{0, Side::Buy, 100, 1, 1}; }, "zero id accepted");
        }},
        {"order rejects zero price", [] {
            check_throws([] { Order{1, Side::Buy, 0, 1, 1}; }, "zero price accepted");
        }},
        {"order rejects zero quantity", [] {
            check_throws([] { Order{1, Side::Buy, 100, 0, 1}; }, "zero quantity accepted");
        }},
        {"fill reduces remaining quantity", [] {
            auto order = buy(1, 100, 10);
            order.fill(4);
            check_equal(order.remaining_quantity(), Quantity{6}, "wrong remaining quantity");
        }},
        {"fill rejects overfill", [] {
            auto order = buy(1, 100, 10);
            check_throws([&] { order.fill(11); }, "overfill accepted");
        }},
        {"add and contains", [] {
            OrderBook book;
            check(book.add_order(buy(1, 100, 10)), "add failed");
            check(book.contains(1), "order missing");
        }},
        {"duplicate add rejected", [] {
            OrderBook book;
            check(book.add_order(buy(1, 100, 10)), "first add failed");
            check(!book.add_order(buy(1, 101, 20)), "duplicate accepted");
        }},
        {"cancel existing order", [] {
            OrderBook book;
            book.add_order(buy(1, 100, 10));
            check(book.cancel_order(1), "cancel failed");
            check(!book.contains(1), "cancelled order remains");
        }},
        {"cancel missing order", [] {
            OrderBook book;
            check(!book.cancel_order(99), "missing cancel succeeded");
        }},
        {"best bid uses highest price", [] {
            OrderBook book;
            book.add_order(buy(1, 100, 10));
            book.add_order(buy(2, 105, 10));
            book.add_order(buy(3, 103, 10));
            check_equal(book.best_bid().value(), Price{105}, "wrong best bid");
        }},
        {"best ask uses lowest price", [] {
            OrderBook book;
            book.add_order(sell(1, 110, 10));
            book.add_order(sell(2, 106, 10));
            book.add_order(sell(3, 108, 10));
            check_equal(book.best_ask().value(), Price{106}, "wrong best ask");
        }},
        {"book size tracks resting orders", [] {
            OrderBook book;
            book.add_order(buy(1, 100, 10));
            book.add_order(sell(2, 110, 10));
            check_equal(book.size(), std::size_t{2}, "wrong size");
        }},
        {"non crossing buy rests", [] {
            OrderBook book;
            (void)book.submit_order(sell(1, 110, 10));
            const auto trades = book.submit_order(buy(2, 109, 5));
            check(trades.empty(), "unexpected trade");
            check(book.contains(2), "buy did not rest");
        }},
        {"non crossing sell rests", [] {
            OrderBook book;
            (void)book.submit_order(buy(1, 100, 10));
            const auto trades = book.submit_order(sell(2, 101, 5));
            check(trades.empty(), "unexpected trade");
            check(book.contains(2), "sell did not rest");
        }},
        {"exact buy match", [] {
            OrderBook book;
            (void)book.submit_order(sell(1, 100, 10));
            const auto trades = book.submit_order(buy(2, 100, 10));
            check_equal(trades.size(), std::size_t{1}, "wrong trade count");
            check_equal(trades[0].quantity, Quantity{10}, "wrong trade quantity");
            check_equal(book.size(), std::size_t{0}, "filled orders remain");
        }},
        {"exact sell match", [] {
            OrderBook book;
            (void)book.submit_order(buy(1, 100, 10));
            const auto trades = book.submit_order(sell(2, 100, 10));
            check_equal(trades.size(), std::size_t{1}, "wrong trade count");
            check_equal(book.size(), std::size_t{0}, "filled orders remain");
        }},
        {"incoming buy partially fills and rests", [] {
            OrderBook book;
            (void)book.submit_order(sell(1, 100, 4));
            const auto trades = book.submit_order(buy(2, 100, 10));
            check_equal(trades[0].quantity, Quantity{4}, "wrong executed quantity");
            check_equal(book.remaining_quantity(2).value(), Quantity{6}, "wrong buy remainder");
        }},
        {"resting ask partially fills", [] {
            OrderBook book;
            (void)book.submit_order(sell(1, 100, 10));
            (void)book.submit_order(buy(2, 100, 4));
            check_equal(book.remaining_quantity(1).value(), Quantity{6}, "wrong ask remainder");
        }},
        {"incoming sell partially fills and rests", [] {
            OrderBook book;
            (void)book.submit_order(buy(1, 100, 4));
            (void)book.submit_order(sell(2, 100, 10));
            check_equal(book.remaining_quantity(2).value(), Quantity{6}, "wrong sell remainder");
        }},
        {"resting bid partially fills", [] {
            OrderBook book;
            (void)book.submit_order(buy(1, 100, 10));
            (void)book.submit_order(sell(2, 100, 4));
            check_equal(book.remaining_quantity(1).value(), Quantity{6}, "wrong bid remainder");
        }},
        {"multi level buy sweep", [] {
            OrderBook book;
            (void)book.submit_order(sell(1, 100, 3));
            (void)book.submit_order(sell(2, 101, 4));
            (void)book.submit_order(sell(3, 102, 5));
            const auto trades = book.submit_order(buy(4, 101, 10));
            check_equal(trades.size(), std::size_t{2}, "wrong number of levels matched");
            check_equal(trades[0].price, Price{100}, "did not match best ask first");
            check_equal(trades[1].price, Price{101}, "wrong second price");
            check_equal(book.remaining_quantity(4).value(), Quantity{3}, "wrong resting remainder");
        }},
        {"multi level sell sweep", [] {
            OrderBook book;
            (void)book.submit_order(buy(1, 102, 3));
            (void)book.submit_order(buy(2, 101, 4));
            (void)book.submit_order(buy(3, 100, 5));
            const auto trades = book.submit_order(sell(4, 101, 10));
            check_equal(trades.size(), std::size_t{2}, "wrong number of levels matched");
            check_equal(trades[0].price, Price{102}, "did not match best bid first");
            check_equal(trades[1].price, Price{101}, "wrong second price");
            check_equal(book.remaining_quantity(4).value(), Quantity{3}, "wrong sell remainder");
        }},
        {"fifo at same ask price", [] {
            OrderBook book;
            (void)book.submit_order(sell(10, 100, 2, 1));
            (void)book.submit_order(sell(11, 100, 2, 2));
            const auto trades = book.submit_order(buy(12, 100, 3, 3));
            check_equal(trades.size(), std::size_t{2}, "wrong trade count");
            check_equal(trades[0].maker_order_id, OrderId{10}, "FIFO first maker wrong");
            check_equal(trades[1].maker_order_id, OrderId{11}, "FIFO second maker wrong");
        }},
        {"fifo at same bid price", [] {
            OrderBook book;
            (void)book.submit_order(buy(10, 100, 2, 1));
            (void)book.submit_order(buy(11, 100, 2, 2));
            const auto trades = book.submit_order(sell(12, 100, 3, 3));
            check_equal(trades[0].maker_order_id, OrderId{10}, "FIFO first maker wrong");
            check_equal(trades[1].maker_order_id, OrderId{11}, "FIFO second maker wrong");
        }},
        {"trade executes at resting order price", [] {
            OrderBook book;
            (void)book.submit_order(sell(1, 100, 2));
            const auto trades = book.submit_order(buy(2, 105, 2));
            check_equal(trades[0].price, Price{100}, "wrong execution price");
        }},
        {"equal prices cross", [] {
            OrderBook book;
            (void)book.submit_order(sell(1, 100, 1));
            const auto trades = book.submit_order(buy(2, 100, 1));
            check_equal(trades.size(), std::size_t{1}, "equal price did not cross");
        }},
        {"one tick apart does not cross", [] {
            OrderBook book;
            (void)book.submit_order(sell(1, 101, 1));
            const auto trades = book.submit_order(buy(2, 100, 1));
            check(trades.empty(), "non-crossing prices traded");
        }},
        {"filled price level disappears", [] {
            OrderBook book;
            (void)book.submit_order(sell(1, 100, 2));
            (void)book.submit_order(sell(2, 101, 2));
            (void)book.submit_order(buy(3, 100, 2));
            check_equal(book.best_ask().value(), Price{101}, "empty price level remains");
        }},
        {"cancel works after partial fill", [] {
            OrderBook book;
            (void)book.submit_order(sell(1, 100, 10));
            (void)book.submit_order(buy(2, 100, 4));
            check(book.cancel_order(1), "cancel after partial fill failed");
            check(!book.contains(1), "partially filled order remains");
        }},
        {"snapshot preserves FIFO visibility", [] {
            OrderBook book;
            book.add_order(buy(1, 100, 2, 1));
            book.add_order(buy(2, 100, 3, 2));
            std::ostringstream out;
            book.print_snapshot(out);
            const auto text = out.str();
            check(text.find("ID: 1") < text.find("ID: 2"), "snapshot FIFO order wrong");
        }},
        {"parser accepts valid add", [] {
            std::string error;
            const auto command = parse_command("ADD 7 BUY 100 5", error);
            check(command.has_value(), "valid ADD rejected");
            check_equal(command->id, OrderId{7}, "parsed id wrong");
        }},
        {"parser rejects invalid side", [] {
            std::string error;
            const auto command = parse_command("ADD 7 HOLD 100 5", error);
            check(!command.has_value(), "invalid side accepted");
        }},
    };

    std::size_t passed = 0;

    for (const auto& [name, test] : tests)
    {
        try
        {
            test();
            ++passed;
            std::cout << "[PASS] " << name << '\n';
        }
        catch (const std::exception& exception)
        {
            std::cerr << "[FAIL] " << name
                      << ": " << exception.what()
                      << '\n';
        }
    }

    std::cout << "\n" << passed << "/"
              << tests.size() << " tests passed\n";

    return passed == tests.size() ? 0 : 1;
}
