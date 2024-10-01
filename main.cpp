#include <iostream>

#include "configuration.hpp"
#include "session.hpp"
#include "marketdata.hpp"
#include "partial_book_update.hpp"
#include "storage.hpp"

#include <string>

int main()
{
    Configuration configuration;
    configuration.host = "stream.binance.com";
    configuration.port = "9443";

    const std::vector<std::string> symbols = {"btcfdusd", "ethfdusd", "adausdt", "avaxusdt"};
    Storage storage{symbols};
    Marketdata marketdata{configuration, symbols};
    auto callback = [&storage](const std::string& response) {
        // std::cout << response << std::endl;
        auto update = PartialBookUpdate::FromJson(response);
        if (update) {
            storage.Update(*update);
            // auto best_bid = update->GetBestBid();
            // std::cout << "best_bid price: " << best_bid.price << ", quantity:" << best_bid.quantity << std::endl;
            // auto best_ask = update->GetBestAsk();
            // std::cout << "best_ask price: " << best_ask.price << ", quantity:" << best_ask.quantity << std::endl;
            // std::cout << "Bid VWAP: " << update->CalculateBidVWAP() << std::endl;
            // std::cout << "Ask VWAP: " << update->CalculateAskVWAP() << std::endl;
        }
        
    };
    marketdata.start(callback);
    std::cin.get();
    return 0;
}
