#pragma once

#include "data_store.h"
#include <optional>
#include <string>
#include <utility>
#include <vector>

class IAggregator {
public:
    virtual ~IAggregator() = default;
    [[nodiscard]] virtual double compute(const std::vector<double>& values) const = 0;
    [[nodiscard]] virtual std::string name() const = 0;
};

class SumAggregator final : public IAggregator {
public:
    [[nodiscard]] double compute(const std::vector<double>& values) const override;
    [[nodiscard]] std::string name() const override { return "SUM"; }
};

class AvgAggregator final : public IAggregator {
public:
    [[nodiscard]] double compute(const std::vector<double>& values) const override;
    [[nodiscard]] std::string name() const override { return "AVG"; }
};

class MaxAggregator final : public IAggregator {
public:
    [[nodiscard]] double compute(const std::vector<double>& values) const override;
    [[nodiscard]] std::string name() const override { return "MAX"; }
};

class VarianceAggregator final : public IAggregator {
public:
    [[nodiscard]] double compute(const std::vector<double>& values) const override;
    [[nodiscard]] std::string name() const override { return "VARIANCE"; }
};

class AnalyticsEngine {
public:
    [[nodiscard]] static std::vector<std::pair<std::string, double>> parallel_group_by_dept(
        const ColumnarDataStore& store,
        const IAggregator& aggregator,
        size_t num_threads = 4
    );
    [[nodiscard]] static double compute_profit_margin_pct(const ColumnarDataStore& store);
    [[nodiscard]] static std::optional<double> forecast_next_period_ols(const std::vector<double>& series);
};