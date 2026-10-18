#include <iostream>
#include <cassert>
#include <cmath>
#include <memory>
#include "data_store.h"
#include "analytics_engine.h"

int main() {
    ColumnarDataStore store;
    store.ingest_csv_line("20260101,Europe,Banking,1000.0,400.0");
    store.ingest_csv_line("20260115,Asia,Retail,2500.0,1000.0");
    store.ingest_csv_line("20260201,Europe,Banking,3000.0,1200.0");

    // Test 1: Row count
    assert(store.row_count() == 3);

    // Test 2: Date range query
    std::vector<double> jan_rev = store.get_revenues_by_date_range(20260101, 20260131);
    assert(jan_rev.size() == 2);
    assert( std::abs(jan_rev[0] - 1000) < 1e-6 );
    assert( std::abs(jan_rev[1] - 2500) < 1e-6 );

    // Test 3: Department Index lookup
    assert(store.get_dept_index().at("Banking").size() == 2);

    // Test 4: Polymorphic Aggregators & Multithreaded Group-By
    const std::unique_ptr<IAggregator> sum_agg = std::make_unique<SumAggregator>();
    const auto grouped = AnalyticsEngine::parallel_group_by_dept(store, *sum_agg, 2);
    assert(grouped.size() == 2);
    assert(grouped[0].first == "Banking");
    assert(std::abs(grouped[0].second - 4000.0) < 1e-6);

    // Test 5: Profit Margin ((6500 - 2600) / 6500 * 100 = 60.0%)
    assert(std::abs(AnalyticsEngine::compute_profit_margin_pct(store) - 60.0) < 1e-6);

    // Test 6: OLS Linear Regression Forecast (100, 200, 300, 400 -> Prediction is 500)
    const auto forecast = AnalyticsEngine::forecast_next_period_ols({100.0, 200.0, 300.0, 400.0});
    assert(forecast.has_value());
    assert(std::abs(*forecast - 500.0) < 1e-6);

    std::cout << "[PASS] All Unit Tests Passed!\n";
    return 0;
}