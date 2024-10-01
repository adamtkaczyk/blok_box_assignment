#include "storage.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <ctime>
#include <sstream>

namespace {
std::string GetSymbol(const std::string& input) {
    size_t pos = input.find('@');
    if (pos != std::string::npos) {
        return input.substr(0, pos);
    }
    return "";
}
}

Storage::Storage(const std::vector<std::string>& streams) {
    for (const auto& stream : streams) {
        storage_map_.emplace(stream, StorageItem());
    }
}

void Storage::Update(const PartialBookUpdate& partial_book_update) {
    std::unique_lock<std::shared_mutex> lock(mtx_);
    auto stream = partial_book_update.GetStream();
    auto it = storage_map_.find(GetSymbol(stream));
    if (it != storage_map_.end()) {
        it->second.Update(partial_book_update);
    }
}

std::pair<BestPrice, BestPrice> Storage::Get(const std::string& stream) const {
    std::shared_lock<std::shared_mutex> lock(mtx_);
    auto it = storage_map_.find(stream);
    if (it != storage_map_.end()) {
        return it->second.Get();
    }
    return {{0, 0}, {0, 0}};
}

void Storage::PriceManager() const {
    std::vector<std::tuple<std::string, double, double>> data_to_write;
    {
        std::shared_lock<std::shared_mutex> lock(mtx_);
        for (const auto& [stream, storage_item] : storage_map_) {
            auto [avg_bid_vwap, avg_ask_vwap] = storage_item.CalculateMovingAverage();
            data_to_write.emplace_back(stream, avg_bid_vwap / static_cast<double>(SCALE_FACTOR), avg_ask_vwap / static_cast<double>(SCALE_FACTOR));
        }
    }

    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << "VWAP_Report_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".csv";

    std::ofstream file(oss.str());
    if (!file.is_open()) {
        std::cerr << "Can't open file: " << oss.str() << std::endl;
        return;
    }

    file << "Stream,Bid_VWAP,Ask_VWAP\n";
    for (const auto& [stream, avg_bid_vwap, avg_ask_vwap] : data_to_write) {
        file << stream << "," << avg_bid_vwap << "," << avg_ask_vwap << "\n";
    }

    file.close();
}