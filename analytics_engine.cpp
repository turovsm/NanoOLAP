#include "analytics_engine.h"

#include <algorithm>
#include <numeric>
#include <thread>
#include <vector>

double SumAggregator::compute(const std::vector<double>& values) const {
    return std::accumulate(values.begin(), values.end(), 0.0);
}

double AvgAggregator::compute(const std::vector<double>& values) const {
    if (values.empty()) return 0.0;
    return SumAggregator{}.compute(values) / static_cast<double>(values.size());
}

double MaxAggregator::compute(const std::vector<double>& values) const {
    if (values.empty()) return 0.0;
    return *std::ranges::max_element(values);
}

double VarianceAggregator::compute(const std::vector<double>& values) const {
    if (values.size() < 2) return 0.0;
    const double mean = AvgAggregator{}.compute(values);
    double sq_diff_sum = 0.0;
    for (const double v : values) {
        const double diff = v - mean;
        sq_diff_sum += diff * diff;
    }
    return sq_diff_sum / static_cast<double>(values.size());
}

std::vector<std::pair<std::string, double>> AnalyticsEngine::parallel_group_by_dept(
    const ColumnarDataStore& store,
    const IAggregator& aggregator,
    size_t num_threads
) {
    const auto& dept_index = store.get_dept_index();
    if (dept_index.empty()) return {};

    const auto& all_revenues = store.get_revenues();

    using MapEntry = std::pair<const std::string, std::vector<size_t>>;
    std::vector<const MapEntry*> entries;
    entries.reserve(dept_index.size());
    for (const auto& entry : dept_index) {
        entries.push_back(&entry);
    }

    if (num_threads == 0) num_threads = 1;
    num_threads = std::min(num_threads, entries.size());

    std::vector<std::pair<std::string, double>> results(entries.size());

    {
        std::vector<std::jthread> workers;
        workers.reserve(num_threads);

        for (size_t t = 0; t < num_threads; ++t) {
            workers.emplace_back([t, num_threads, &entries, &all_revenues, &aggregator, &results]() {
                std::vector<double> dept_revs;
                for (size_t i = t; i < entries.size(); i += num_threads) {
                    const auto& [dept, row_indices] = *entries[i];

                    dept_revs.clear();
                    dept_revs.reserve(row_indices.size());
                    for (const size_t idx : row_indices) {
                        dept_revs.push_back(all_revenues[idx]);
                    }

                    results[i] = {dept, aggregator.compute(dept_revs)};
                }
            });
        }
    }

    std::ranges::sort(results, [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    return results;
}

double AnalyticsEngine::compute_profit_margin_pct(const ColumnarDataStore& store) {
    const auto& all_revenues = store.get_revenues();
    const auto& all_costs = store.get_costs();
    const double total_revenue = SumAggregator{}.compute(all_revenues);
    if (total_revenue == 0.0) return 0.0;
    const double total_cost = SumAggregator{}.compute(all_costs);
    return ((total_revenue - total_cost) / total_revenue) * 100.0;
}

std::optional<double> AnalyticsEngine::forecast_next_period_ols(const std::vector<double>& series) {
    if (series.size() < 2) return std::nullopt;
    const auto n = static_cast<double>(series.size());
    const double mean_x = (n - 1.0) / 2.0;
    const double mean_y = AvgAggregator{}.compute(series);

    double num = 0.0;
    double den = 0.0;
    for (size_t i = 0; i < series.size(); ++i) {
        const double dx = static_cast<double>(i) - mean_x;
        const double dy = series[i] - mean_y;
        num += dx * dy;
        den += dx * dx;
    }
    if (den == 0.0) return mean_y;
    const double slope = num / den;
    const double intercept = mean_y - slope * mean_x;
    return slope * n + intercept;
}