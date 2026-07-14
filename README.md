# Chronos

Chronos is a C++20 limit order book and matching engine project.

The project is being developed incrementally to explore:

- Modern C++
- Data structures
- Financial exchange mechanics
- Testing
- Performance benchmarking

## Current functionality

- Validated order model
- Integer-based prices
- Buy and sell order sides

## Build

```bash
cmake -S . -B build
cmake --build build
./build/chronos