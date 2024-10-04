#pragma once

#include <vector>
#include <optional>
#include <string>

const long long SCALE_FACTOR = 100000000;

struct BestPrice {
    long long price;
    long long quantity;
};

class PartialBookUpdate {
public:
    PartialBookUpdate(const std::string& stream, const std::vector<std::pair<long long, long long>>& bids, const std::vector<std::pair<long long, long long>>& asks);
    PartialBookUpdate(std::string&& stream, std::vector<std::pair<long long, long long>>&& bids, std::vector<std::pair<long long, long long>>&& asks);

    std::string GetStream() const;
    BestPrice GetBestBid() const;
    BestPrice GetBestAsk() const;

    long long CalculateBidVWAP() const;
    long long CalculateAskVWAP() const;

    static std::optional<PartialBookUpdate> FromJson(const std::string& json_string);
private:
    std::string stream_;
    std::vector<std::pair<long long, long long>> bids_;
    std::vector<std::pair<long long, long long>> asks_;

    void ParseJson(const std::string& json_string);
    long long CalculateVWAP(const std::vector<std::pair<long long, long long>>& prices) const;
};
