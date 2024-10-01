#include "marketdata.hpp"

Marketdata::Marketdata(Configuration configuration, const std::vector<std::string>& symbols) 
    : configuration_(configuration)
    , symbols_(symbols)
    , evnet_work_(boost::asio::make_work_guard(event_ioc_))
    , event_net_thread_([&ioc = event_ioc_]() { ioc.run(); }) {
        sessions_.reserve(symbols.size());
}

void Marketdata::startInSepetateSession() {
    for (const auto& symbol : symbols_) {
        auto callback = [](const std::string& response) {
            printf("response: %s\n", response.c_str()); 
        };
        std::string subscribe_request = R"({
            "method": "SUBSCRIBE",
            "params":
            [
            ")" + symbol + R"(@depth10@100ms"
            ],
            "id": )" + std::to_string(sessions_.size() + 1) + R"(
        })";

        std::string unsubscribe_request = R"({
            "method": "UNSUBSCRIBE",
            "params":
            [
            ")" + symbol + R"("
            ],
            "id": )" + std::to_string(sessions_.size() + 1) + R"(
            })";

        auto websocket_path = "/ws/" + symbol + "@depth10@100ms";

        auto session = std::make_shared<Session>(
                configuration_,
                subscribe_request,
                unsubscribe_request,
                websocket_path,
                event_ioc_,
                callback);

        auto subscription_future = session->start();
        subscription_future.get();
        sessions_.push_back(std::move(session));
    }
}

void Marketdata::start(const std::function<void(const std::string&)>& callback) {
    std::string symbols = std::accumulate(std::begin(symbols_), std::end(symbols_), std::string(),
        [](std::string &ss, std::string &s)
        {
            auto symbol = "\"" + s + "@depth10@100ms\"";
            return  ss.empty() ? symbol : ss + "," + symbol; 
        });
    std::string stream = "/stream?streams=" + std::accumulate(std::begin(symbols_), std::end(symbols_), std::string(),
        [](std::string &ss, std::string &s)
        {
            auto symbol = s + "@depth10@100ms";
            return ss.empty() ? symbol : ss + "/" + symbol;
        });

    std::string subscribe_request = R"({
        "method": "SUBSCRIBE",
        "params":
        [)" + symbols + R"(],
        "id": 1})";

    std::string unsubscribe_request = R"({
        "method": "UNSUBSCRIBE",
        "params":
        [)" + symbols + R"(],
        "id": 1})";

    session_ = std::make_shared<Session>(
                configuration_,
                subscribe_request,
                unsubscribe_request,
                stream,
                event_ioc_,
                callback);

    auto subscription_future = session_->start();
    subscription_future.get();
}

void Marketdata::stop() {
    for (const auto& session : sessions_) {
        auto unsubscription_future = session->stop();
        unsubscription_future.get();
    }
    sessions_.clear();
}

Marketdata::~Marketdata() {
    event_ioc_.stop();
    event_net_thread_.join();
}