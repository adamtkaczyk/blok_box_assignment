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
        auto update = PartialBookUpdate::FromJson(response);
        if (update) {
            storage.Update(*update);
        }
        
    };
    marketdata.start(callback);
    
    std::thread background_thread([&storage, &symbols]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::minutes(2));

            storage.PriceManager();

            for (const auto& symbol : symbols) {
                auto [best_bid, best_ask] = storage.Get(symbol);
                std::cout << "Symbol: " << symbol << std::endl;
                std::cout << "Best Bid - Price: " << static_cast<double>(best_bid.price) / SCALE_FACTOR
                          << ", Quantity: " << static_cast<double>(best_bid.quantity) / SCALE_FACTOR << std::endl;
                std::cout << "Best Ask - Price: " << static_cast<double>(best_ask.price) / SCALE_FACTOR
                          << ", Quantity: " << static_cast<double>(best_ask.quantity) / SCALE_FACTOR << std::endl;
                std::cout << "-------------------------------------" << std::endl;
            }
        }
    });

    std::cin.get();

    background_thread.join();
    return 0;
}
