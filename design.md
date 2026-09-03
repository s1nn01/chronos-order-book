# Chronos Order Book Design

## Purpose

The order book stores buy and sell orders for one financial instrument.

During the first implementation, the system will support:

* Adding limit orders
* Cancelling orders
* Looking up the best bid
* Looking up the best ask
* Maintaining price-time priority

Order matching will be added after the storage operations work correctly.

## Price-time priority

Orders are prioritised using two rules:

1. The order with the best price receives priority.
2. Orders at the same price are prioritised by arrival time.

### Buy orders

A higher buy price has greater priority.

Example:

```text
BUY 100 shares at 10500
BUY 100 shares at 10600
```

The order at `10600` has priority because the buyer is willing to pay more.

### Sell orders

A lower sell price has greater priority.

Example:

```text
SELL 100 shares at 10700
SELL 100 shares at 10650
```

The order at `10650` has priority because the seller is offering a lower price.

### Orders at the same price

Orders with the same price are processed in arrival order.

Example:

```text
Order 1: BUY 50 at 10500
Order 2: BUY 30 at 10500
```

Order 1 has priority because it arrived first.

## Data structures

Buy and sell price levels will be stored separately.

```text
Bids: highest price first
Asks: lowest price first
```

Each price level contains a FIFO list of orders.

The baseline implementation uses:

* `std::map` to keep price levels sorted
* `std::list` to preserve order arrival sequence
* `std::unordered_map` to locate an order quickly by ID

## Important invariants

The order book must always maintain these rules:

* Every order ID is unique.
* An order belongs to exactly one price level.
* Buy prices are ordered from highest to lowest.
* Sell prices are ordered from lowest to highest.
* Orders at the same price remain in arrival order.
* Empty price levels are removed.
* Cancelling an unknown order does not modify the book.

## Matching rules

Incoming limit orders first attempt to match against the best prices on the opposite side.

- An incoming buy can execute while `best_ask <= buy_limit_price`.
- An incoming sell can execute while `best_bid >= sell_limit_price`.
- Execution occurs at the resting (maker) order's price.
- If quantities differ, the smaller quantity is executed and the larger order is partially filled.
- A fully filled resting order is removed from both its FIFO price level and the order-ID index.
- Empty price levels are erased.
- If an incoming limit order still has quantity after all eligible matches, its remainder rests on the book with its original sequence number.

### Complexity baseline

| Operation | Baseline structure | Expected complexity |
| --- | --- | --- |
| Best bid / ask | first element of sorted `std::map` | O(1) after map lookup state is maintained |
| New price level | `std::map` insertion | O(log P) |
| Append at existing level | `std::list::push_back` | O(1) |
| Order-ID lookup | `std::unordered_map` | O(1) average |
| Cancel located order | stable `std::list` iterator + map lookup | O(1) average plus O(log P) if the price level becomes empty |
| Match | walk only crossed resting orders | proportional to number of fills, with level erasure costs |

`P` is the number of active price levels. This is a correctness-first baseline rather than a claim that the STL layout is optimal for production low-latency trading.
