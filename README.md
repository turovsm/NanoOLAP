# NanoOLAP

An in-memory columnar data store and analytical query engine written in C++20. It parses CSV financial data, indexes records by time and category, runs parallel aggregations across threads, and predicts next-period revenue using linear regression.

## Features
* **Columnar Storage:** Stores table columns in separate `std::vector` buffers (`data_store.h`).
* **CSV Parsing:** Uses `std::string_view` and `std::from_chars` to parse lines without extra string allocations.
* **Indexing:** Uses `std::map` for $O(\log N)$ date-range lookups and `std::unordered_map` for $O(1)$ department lookups.
* **Parallel Aggregations:** Computes `SUM`, `AVG`, `MAX`, and `VARIANCE` grouped by department across multiple `std::jthread` workers (`analytics_engine.h`).
* **Forecasting:** Predicts next-period revenue using Ordinary Least Squares (OLS) linear regression.

## Requirements
* C++20 compiler (GCC 13+, Clang 16+, or MSVC 2022)
* CMake 3.20+ 

## Build & Run
From the project root directory (`NanoOLAP/`):

```bash

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# Run unit tests
./nano_olap_tests

# Run benchmark
./nano_olap

```

## Benchmark (250,000 Rows, Release Build)

| Task | Time |
| --- | --- |
| CSV Ingestion & Indexing (250,000 rows) | `24.02 ms` |
| Group-By `SUM` (4 threads) | `0.57 ms` |
| Group-By `AVG` (4 threads) | `0.30 ms` |
| Group-By `MAX` (4 threads) | `0.38 ms` |
| Group-By `VARIANCE` (4 threads) | `0.42 ms` |

