#include "order.hpp"
#include "order_book.hpp"
#include "parser.hpp"
#include "trade.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

void print_top_of_book(
    const OrderBook& order_book)
{
    std::cout << "Book size: "
              << order_book.size()
              << '\n';

    std::cout << "Best bid: ";

    if (const auto bid = order_book.best_bid();
        bid.has_value())
    {
        std::cout << bid.value();
    }
    else
    {
        std::cout << "NONE";
    }

    std::cout << '\n';

    std::cout << "Best ask: ";

    if (const auto ask = order_book.best_ask();
        ask.has_value())
    {
        std::cout << ask.value();
    }
    else
    {
        std::cout << "NONE";
    }

    std::cout << "\n\n";
}

void print_trades(const std::vector<Trade>& trades)
{
    for (const Trade& trade : trades)
    {
        std::cout
            << "TRADE #" << trade.sequence_number
            << " maker=" << trade.maker_order_id
            << " taker=" << trade.taker_order_id
            << " price=" << trade.price
            << " quantity=" << trade.quantity
            << '\n';
    }
}

int main()
{
    std::cout << "Chronos C++ Matching Engine\n";
    std::cout << "Commands:\n";
    std::cout
        << "  ADD <id> <BUY|SELL> "
        << "<price> <quantity>\n";
    std::cout << "  CANCEL <id>\n";
    std::cout << "  EXIT\n\n";

    OrderBook order_book;

    std::uint64_t next_sequence = 1;
    std::string line;

    while (true)
    {
        std::cout << "> ";

        if (!std::getline(std::cin, line))
        {
            break;
        }

        std::string error;

        const auto command =
            parse_command(line, error);

        if (!command.has_value())
        {
            std::cerr << "Error: "
                      << error
                      << '\n';

            continue;
        }

        switch (command->type)
        {
            case CommandType::Add:
            {
                try
                {
                    if (order_book.contains(command->id))
                    {
                        std::cerr
                            << "Error: order ID "
                            << command->id
                            << " already exists\n";
                        break;
                    }

                    const Order order(
                        command->id,
                        command->side,
                        command->price,
                        command->quantity,
                        next_sequence++
                    );

                    const auto trades =
                        order_book.submit_order(order);

                    std::cout
                        << "Accepted: "
                        << order.to_string()
                        << '\n';

                    print_trades(trades);
                    print_top_of_book(order_book);
                    order_book.print_snapshot(std::cout);
                }
                catch (
                    const std::exception& exception)
                {
                    std::cerr
                        << "Error: "
                        << exception.what()
                        << '\n';
                }

                break;
            }

            case CommandType::Cancel:
            {
                if (order_book.cancel_order(
                        command->id))
                {
                    std::cout
                        << "Cancelled order "
                        << command->id
                        << '\n';

                    print_top_of_book(order_book);
                    order_book.print_snapshot(std::cout);
                }
                else
                {
                    std::cerr
                        << "Error: order "
                        << command->id
                        << " was not found\n";
                }

                break;
            }

            case CommandType::Exit:
            {
                std::cout
                    << "Chronos shutting down.\n";

                return 0;
            }
        }
    }

    return 0;
}
