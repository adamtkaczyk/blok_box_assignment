#pragma once

#include "partial_book_update.hpp"

#include <vector>
#include <shared_mutex>

class StorageItem {
public:
    StorageItem(size_t size = 100);
    StorageItem(StorageItem&& other);
    StorageItem& operator=(StorageItem&& other);

    void Update(const PartialBookUpdate& partial_book_update);
    std::pair<BestPrice, BestPrice> Get() const;
    std::pair<long long, long long> CalculateMovingAverage() const;
private:
    BestPrice best_bid_;
    BestPrice best_ask_;

    std::vector<long long> vwap_bid_history_;
    std::vector<long long> vwap_ask_history_;

    size_t current_index_;
    size_t updates_counter_;
    size_t size_;
    mutable std::shared_mutex mtx_;

    long long CalculateMovingAverage(const std::vector<long long>& history) const;
};