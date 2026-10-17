#include <iostream>
#include <cassert>
#include <cmath>
#include "data_store.h"

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

    std::cout << "[PASS] All Unit Tests Passed!\n";
    return 0;
}