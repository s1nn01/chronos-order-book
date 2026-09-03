# Chronos

A C++20 limit order book and matching engine, built from scratch to explore
exchange mechanics, low-latency data-structure design, and systematic testing.

**Status:** in active development. The order-storage core (insertion,
cancellation, price-time priority, best bid/ask) is implemented and working;
the matching engine, partial fills, test suite and latency benchmarks are the
current focus (see [Roadmap](#roadmap)).

## What it does today

- **Price-time priority order book** for a single instrument, maintaining bids
  highest-price-first and asks lowest-price-first, with FIFO ordering within
  each price level.
- **Order insertion and cancellation** in logarithmic time on the number of
  price levels, with O(1) lookup of any resting order by ID.
- **Best bid / best ask** queries returning `std::optional<Price>`.
- **Book snapshots** printing aggregate quantity and per-order detail at each
  price level, for inspection and debugging.

## Design

The order book is built on three cooperating structures:

| Structure | Type | Role |
|-----------|------|------|
| Bid levels | `std::map<Price, Queue, std::greater<>>` | price levels, best (highest) bid first |
| Ask levels | `std::map<Price, Queue, std::less<>>` | price levels, best (lowest) ask first |
| Order index | `std::unordered_map<OrderId, OrderLocation>` | O(1) locate-by-ID for cancellation |

Each price level holds a `std::list<Order>` so that orders at the same price
keep their arrival order (time priority) and cancellation is O(1) given an
iterator from the index. The invariants the book maintains — unique IDs, one
level per order, empty levels pruned, arrival order preserved — are written up
in [`design.md`](design.md).

The code targets C++20 with warnings treated seriously (`-Wall -Wextra
-Wpedantic`, `/W4 /permissive-` on MSVC) and uses `[[nodiscard]]`, `noexcept`
and `std::optional` throughout.

## Build

Requires CMake ≥ 3.20 and a C++20 compiler.

```bash
cmake -S . -B build
cmake --build build
./build/chronos
```

## Roadmap

- [ ] **Matching engine** — cross incoming orders against the opposite side by
      price-time priority, generating trades.
- [ ] **Partial fills** — decrement resting order quantity on partial matches;
      remove fully-filled orders.
- [ ] **Test suite** — unit tests covering FIFO priority, cancellation of known
      and unknown IDs, empty-level pruning, and matching invariants.
- [ ] **Benchmark harness** — drive the book with synthetic order streams and
      report matching throughput and p50/p99 latency across varying book depths.

## Project layout

```
include/    public headers (order, order_book, parser, command types)
src/        implementation and CLI entry point
design.md   data-structure and invariant design notes
```
