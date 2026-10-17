#include "data_store.h"

#include <charconv>
#include <utility>
#include <type_traits>

namespace {
    std::string_view next_token(std::string_view& line) {
        const size_t comma_pos = line.find(',');
        if (comma_pos == std::string_view::npos) {
            return std::exchange(line, std::string_view{});
        }
        return std::exchange(line, line.substr(comma_pos + 1)).substr(0, comma_pos);
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    T parse_number(std::string_view sv) {
        T value{};
        std::from_chars(sv.data(), sv.data() + sv.size(), value);
        return value;
    }
}

void ColumnarDataStore::ingest_csv_line(std::string_view line) {
    if (line.ends_with('\r')) line.remove_suffix(1);
    if (line.empty()) return;

    const size_t current_row_idx = revenues_.size();

    const int timestamp = parse_number<int>(next_token(line));
    const std::string_view region = next_token(line);
    std::string dept(next_token(line));
    const auto revenue = parse_number<double>(next_token(line));
    const auto cost = parse_number<double>(next_token(line));

    timestamps_.push_back(timestamp);
    regions_.emplace_back(region);
    departments_.push_back(dept);
    revenues_.push_back(revenue);
    costs_.push_back(cost);

    time_index_[timestamp].push_back(current_row_idx);
    dept_index_[std::move(dept)].push_back(current_row_idx);
}

size_t ColumnarDataStore::row_count() const {
    return revenues_.size();
}

std::vector<double> ColumnarDataStore::get_revenues_by_date_range(int start_date, int end_date) const {
    if (start_date > end_date) return {};

    std::vector<double> result;
    for (auto it = time_index_.lower_bound(start_date);
        it != time_index_.end() && it->first <= end_date;
        ++it) {
        for (const size_t row_idx : it->second) {
            result.push_back(revenues_[row_idx]);
        }
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
