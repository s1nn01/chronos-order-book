#include "order.hpp"
#include "parser.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

int main()
{
    std::cout << "Chronos C++ Matching Engine\n";
    std::cout << "Commands:\n";
    std::cout << "  ADD <id> <BUY|SELL> <price> <quantity>\n";
    std::cout << "  CANCEL <id>\n";
    std::cout << "  EXIT\n\n";

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
        const auto command = parse_command(line, error);

        if (!command.has_value())
        {
            std::cerr << "Error: " << error << '\n';
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
                        next_sequence++
                    );

                    std::cout << "Accepted: "
                              << order.to_string()
                              << '\n';
                }
                catch (const std::exception& exception)
                {
                    std::cerr << "Error: "
                              << exception.what()
                              << '\n';
                }

                break;
            }

            case CommandType::Cancel:
            {
                std::cout
                    << "Cancellation requested for order "
                    << command->id
                    << '\n';

                break;
            }

            case CommandType::Exit:
            {
                std::cout << "Chronos shutting down.\n";
                return 0;
            }
        }
    }

    return 0;
}