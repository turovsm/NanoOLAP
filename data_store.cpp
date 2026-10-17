#include "data_store.h"

static std::string_view next_token(std::string_view& line) {
    const size_t comma_pos = line.find(',');
    if (comma_pos == std::string_view::npos) {
        const std::string_view token = line;
        line = "";
        return token;
    }
    const std::string_view token = line.substr(0, comma_pos);
    line = line.substr(comma_pos + 1);
    return token;
}

void ColumnarDataStore::ingest_csv_line(std::string_view line) {
    if (line.empty()) return;

    const size_t current_row_idx = revenues_.size();

    const int timestamp = std::stoi(std::string(next_token(line)));
    const std::string region(next_token(line));
    const std::string dept(next_token(line));
    const double revenue = std::stod(std::string(next_token(line)));
    const double cost = std::stod(std::string(next_token(line)));

    timestamps_.push_back(timestamp);
    regions_.push_back(region);
    departments_.push_back(dept);
    revenues_.push_back(revenue);
    costs_.push_back(cost);

    time_index_[timestamp].push_back(current_row_idx);
    dept_index_[dept].push_back(current_row_idx);
}

size_t ColumnarDataStore::row_count() const {
    return revenues_.size();
}

std::vector<double> ColumnarDataStore::get_revenues_by_date_range(int start_date, int end_date) const {
    std::vector<double> result;
    auto it = time_index_.lower_bound(start_date);
    while (it != time_index_.end() && it->first <= end_date) {
        for (const size_t row_idx : it->second) {
            result.push_back(revenues_[row_idx]);
        }
        ++it;
    }
    return result;
}

const std::unordered_map<std::string, std::vector<size_t>>& ColumnarDataStore::get_dept_index() const {
    return dept_index_;
}

const std::vector<double>& ColumnarDataStore::get_revenues() const {
    return revenues_;
}

const std::vector<double>& ColumnarDataStore::get_costs() const {
    return costs_;
}