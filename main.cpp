#include "analytics_engine.h"
#include "data_store.h"

#include <array>
#include <chrono>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

int main() {
    std::cout << "=========================================================\n";
    std::cout << "   NanoOLAP: C++20 In-Memory BI & CPM Analytics Engine   \n";
    std::cout << "=========================================================\n\n";

    constexpr size_t NUM_ROWS = 250'000;
    constexpr std::array<int, 6> dates = {20260115, 20260215, 20260315, 20260415, 20260515, 20260615};

    std::vector<std::string> raw_csv_lines;
    raw_csv_lines.reserve(NUM_ROWS);

    for (size_t i = 0; i < NUM_ROWS; ++i) {
        constexpr std::array<std::string_view, 4> regions = {"Europe", "Asia", "Americas", "MENA"};
        constexpr std::array<std::string_view, 6> depts = {
            "Banking", "Government", "Retail", "Energy", "Telecom", "Healthcare"
        };

        const size_t month_idx = i % dates.size();
        const int date = dates[month_idx];
        const std::string_view reg = regions[i % regions.size()];
        const std::string_view dept = depts[i % depts.size()];

        const double rev = 1000.0
                           + static_cast<double>(month_idx * 120)
                           + static_cast<double>((i % 7) * 250)
                           + static_cast<double>((i % 13) * 50);
        const double cost = rev * 0.62;
        raw_csv_lines.push_back(std::format("{},{},{},{:.2f},{:.2f}", date, reg, dept, rev, cost));
    }

    ColumnarDataStore store;
    const auto ingest_start = std::chrono::steady_clock::now();
    for (const std::string &line: raw_csv_lines) {
        store.ingest_csv_line(line);
    }
    const auto ingest_end = std::chrono::steady_clock::now();
    const double ingest_ms = std::chrono::duration<double, std::milli>(ingest_end - ingest_start).count();

    std::cout << "[1] Data Ingestion & Indexing\n";
    std::cout << std::format("\tRows Ingested  : {} records\n", store.row_count());
    std::cout << std::format("\tIngestion time : {:.2f} ms\n", ingest_ms);
    std::cout << std::format("\tNet Margin     : {:.2f}%\n\n", AnalyticsEngine::compute_profit_margin_pct(store));

    std::cout << "[2] Department Aggregation (4 worker threads)\n";
    std::vector<std::unique_ptr<IAggregator> > aggregators;
    aggregators.reserve(4);
    aggregators.push_back(std::make_unique<SumAggregator>());
    aggregators.push_back(std::make_unique<AvgAggregator>());
    aggregators.push_back(std::make_unique<MaxAggregator>());
    aggregators.push_back(std::make_unique<VarianceAggregator>());

    for (const auto &aggregator: aggregators) {
        const auto q_start = std::chrono::steady_clock::now();
        const auto results = AnalyticsEngine::parallel_group_by_dept(store, *aggregator, 4);
        const auto q_end = std::chrono::steady_clock::now();
        const double q_ms = std::chrono::duration<double, std::milli>(q_end - q_start).count();

        std::cout << std::format("\tMetric: {} (computed in {:.2f} ms)\n", aggregator->name(), q_ms);
        for (const auto &[dept, val]: results) {
            std::cout << std::format("\t\t{:<12} : {:.2f}\n", dept, val);
        }
    }
    std::cout << "\n";

    std::cout << "[3] Time-Series Index Slicing & CPM Revenue Forecast (OLS)\n";
    std::vector<double> monthly_totals;
    monthly_totals.reserve(dates.size());

    for (const int date: dates) {
        const SumAggregator sum_aggregator;
        const std::vector<double> month_revs = store.get_revenues_by_date_range(date, date);
        const double total = sum_aggregator.compute(month_revs);
        monthly_totals.push_back(total);
        std::cout << std::format("\tDate {} Total Revenue : ${:.2f}\n", date, total);
    }

    if (const auto next_forecast = AnalyticsEngine::forecast_next_period_ols(monthly_totals); next_forecast.
        has_value()) {
        std::cout << "\t----------------------------------------------------\n";
        std::cout << std::format("\tNext period revenue prediction (OLS) : ${:.2f}\n", *next_forecast);
    }

    return 0;
}
