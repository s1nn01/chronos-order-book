#include "order.hpp"
#include "order_book.hpp"
#include "parser.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

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
                    const Order order(
                        command->id,
                        command->side,
                        command->price,
                        command->quantity,
                        next_sequence
                    );

                    if (!order_book.add_order(order))
                    {
                        std::cerr
                            << "Error: order ID "
                            << command->id
                            << " already exists\n";

                        break;
                    }

                    ++next_sequence;

                    std::cout
                        << "Accepted: "
                        << order.to_string()
                        << '\n';

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