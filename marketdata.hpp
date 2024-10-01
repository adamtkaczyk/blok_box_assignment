#pragma once

#include "session.hpp"
#include "configuration.hpp"

#include <unordered_map>
#include <thread>

class Marketdata {
public:
    Marketdata(Configuration configuration, const std::vector<std::string>& symbols);
    ~Marketdata();

    void start(const std::function<void(const std::string&)>& callback);
    void startInSepetateSession();
    void stop();
private:
    Configuration configuration_;
    std::vector<std::string> symbols_;

    boost::asio::io_context event_ioc_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> evnet_work_;
    std::thread event_net_thread_;
    std::vector<std::shared_ptr<Session>> sessions_;
    std::shared_ptr<Session> session_;
};
