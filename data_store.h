#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <map>
#include <unordered_map>

class ColumnarDataStore {
private:
    std::vector<int> timestamps_;
    std::vector<std::string> regions_;
    std::vector<std::string> departments_;
    std::vector<double> revenues_;
    std::vector<double> costs_;
    std::map<int, std::vector<size_t>> time_index_;
    std::unordered_map<std::string, std::vector<size_t>> dept_index_;

public:
    void ingest_csv_line(std::string_view line);
    [[nodiscard]] size_t row_count() const;
    [[nodiscard]] std::vector<double> get_revenues_by_date_range(int start_date, int end_date) const;
    [[nodiscard]] const std::unordered_map<std::string, std::vector<size_t>>& get_dept_index() const;
    [[nodiscard]] const std::vector<double>& get_revenues() const;
    [[nodiscard]] const std::vector<double>& get_costs() const;
};