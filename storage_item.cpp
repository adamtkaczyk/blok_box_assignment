#include "storage_item.hpp"

#include <fstream>
#include <optional>
#include <utility>
#include <iostream>
#include <iomanip>

#include <boost/multiprecision/cpp_int.hpp>

StorageItem::StorageItem(size_t size) 
    : vwap_bid_history_(size, 0)
    , vwap_ask_history_(size, 0)
    , current_index_(0)
    , updates_counter_(0)
    , size_(size) {}

StorageItem::StorageItem(StorageItem&& other)
        : best_bid_(std::move(other.best_bid_)),
          best_ask_(std::move(other.best_ask_)),
          vwap_bid_history_(std::move(other.vwap_bid_history_)),
          vwap_ask_history_(std::move(other.vwap_ask_history_)),
          current_index_(other.current_index_),
          updates_counter_(other.updates_counter_),
          size_(other.size_) {}

StorageItem& StorageItem::operator=(StorageItem&& other) {
    if (this != &other) {
        best_bid_ = std::move(other.best_bid_);
        best_ask_ = std::move(other.best_ask_);
        vwap_bid_history_ = std::move(other.vwap_bid_history_);
        vwap_ask_history_ = std::move(other.vwap_ask_history_);
        current_index_ = other.current_index_;
        updates_counter_ = other.updates_counter_;
        size_ = other.size_;
    }
    return *this;
}

void StorageItem::Update(const PartialBookUpdate& partial_book_update) {
    std::unique_lock<std::shared_mutex> lock(mtx_);
    best_bid_ = partial_book_update.GetBestBid();
    best_ask_ = partial_book_update.GetBestAsk();

    auto bid_vwap = partial_book_update.CalculateBidVWAP();
    auto ask_vwap = partial_book_update.CalculateAskVWAP();

    vwap_bid_history_[current_index_] = bid_vwap;
    vwap_ask_history_[current_index_] = ask_vwap;

    current_index_ = (current_index_ + 1) % size_;

    ++updates_counter_;
}

std::pair<BestPrice, BestPrice> StorageItem::Get() const {
    std::shared_lock<std::shared_mutex> lock(mtx_);
    return {best_bid_, best_ask_};
}

std::pair<long long, long long> StorageItem::CalculateMovingAverage() const {
    std::shared_lock<std::shared_mutex> lock(mtx_);
    if (updates_counter_ < size_) {
        std::cout << "Not enough updates for moving average calculation.\n";
        return {0, 0};
    }

    return {CalculateMovingAverage(vwap_bid_history_), CalculateMovingAverage(vwap_ask_history_)};
}

long long StorageItem::CalculateMovingAverage(const std::vector<long long>& history) const {
    boost::multiprecision::cpp_int sum = 0;
    for (const auto& vwap : history) {
        sum += vwap;
    }

    boost::multiprecision::cpp_int average = sum / static_cast<boost::multiprecision::cpp_int>(history.size());

    return static_cast<long long>(average);
}