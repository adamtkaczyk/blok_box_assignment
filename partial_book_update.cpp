#include "partial_book_update.hpp"

#include <string>
#include <nlohmann/json.hpp>
#include <boost/multiprecision/cpp_int.hpp>

std::optional<PartialBookUpdate> PartialBookUpdate::FromJson(const std::string& json_string) {
    try {
        nlohmann::json json = nlohmann::json::parse(json_string);

        if (!json.contains("stream") || !json["stream"].is_string()) {
            return std::nullopt;
        }
        if (!json.contains("data") || !json["data"].is_object()) {
            return std::nullopt;
        }

        auto stream = json["stream"];
        const auto& data = json["data"];

        if (!data.contains("bids") || !data["bids"].is_array()) {
            return std::nullopt;
        }

        if (!data.contains("asks") || !data["asks"].is_array()) {
            return std::nullopt;
        }

        std::vector<std::pair<long long, long long>> bids;
        for (const auto& bid : data["bids"]) {
            if (bid.size() != 2 || !bid[0].is_string() || !bid[1].is_string()) {
                return std::nullopt;
            }
            double price = std::stod(bid[0].get<std::string>()) * SCALE_FACTOR;
            double quantity = std::stod(bid[1].get<std::string>()) * SCALE_FACTOR;

            bids.emplace_back(static_cast<long long>(price), static_cast<long long>(quantity));
        }

        std::vector<std::pair<long long, long long>> asks;
        for (const auto& ask : data["asks"]) {
            if (ask.size() != 2 || !ask[0].is_string() || !ask[1].is_string()) {
                return std::nullopt;
            }
            double price = std::stod(ask[0].get<std::string>()) * SCALE_FACTOR;
            double quantity = std::stod(ask[1].get<std::string>()) * SCALE_FACTOR;

            asks.emplace_back(static_cast<long long>(price), static_cast<long long>(quantity));
        }

        return PartialBookUpdate(std::move(stream), std::move(bids), std::move(asks));
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return std::nullopt;
    }
}

PartialBookUpdate::PartialBookUpdate(const std::string& stream, const std::vector<std::pair<long long, long long>>& bids, const std::vector<std::pair<long long, long long>>& asks)
    : stream_(stream), bids_(bids), asks_(asks) {

}

PartialBookUpdate::PartialBookUpdate(std::string&& stream, std::vector<std::pair<long long, long long>>&& bids, std::vector<std::pair<long long, long long>>&& asks)
    : stream_(std::move(stream)), bids_(std::move(bids)), asks_(std::move(asks)) {

}

std::string PartialBookUpdate::GetStream() const {
    return stream_;
}

BestPrice PartialBookUpdate::GetBestBid() const {
    BestPrice best_bid = {0, 0};
    for (const auto& bid : bids_) {
        if (bid.first > best_bid.price) {
            best_bid.price = bid.first;
            best_bid.quantity = bid.second;
        }
    }
    return best_bid;
}

BestPrice PartialBookUpdate::GetBestAsk() const {
    BestPrice best_ask = {std::numeric_limits<long long>::max(), 0};
    for (const auto& ask : asks_) {
        if (ask.first < best_ask.price) {
            best_ask.price = ask.first;
            best_ask.quantity = ask.second;
        }
    }
    return best_ask;
}

long long PartialBookUpdate::CalculateBidVWAP() const {
    return CalculateVWAP(bids_);
}

long long PartialBookUpdate::CalculateAskVWAP() const {
    return CalculateVWAP(asks_);
}

long long PartialBookUpdate::CalculateVWAP(const std::vector<std::pair<long long, long long>>& prices) const {
    boost::multiprecision::cpp_int total_quantity = 0;
    boost::multiprecision::cpp_int total_price = 0;
    
    for (const auto& entry : prices) {
        boost::multiprecision::cpp_int price = entry.first;
        boost::multiprecision::cpp_int quantity = entry.second;
        total_quantity += quantity;
        total_price += price * quantity;
    }
    
    return total_quantity > 0 ? static_cast<long long>(total_price / total_quantity) : 0;
}