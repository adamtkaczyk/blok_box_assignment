#pragma once

#include "partial_book_update.hpp"
#include "storage_item.hpp"

#include <vector>
#include <shared_mutex>
#include <unordered_map>

class Storage {
public:
    Storage(const std::vector<std::string>& streams);

    void Update(const PartialBookUpdate& partial_book_update);
    std::pair<BestPrice, BestPrice> Get(const std::string& stream) const;
    void PriceManager() const;

private:
    std::unordered_map<std::string, StorageItem> storage_map_;
    mutable std::shared_mutex mtx_;
};