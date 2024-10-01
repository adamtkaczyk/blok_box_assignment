#pragma once

#include "configuration.hpp"

#include <cstdlib>
#include <functional>
#include <future>
#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace http = beast::http;            // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
namespace net = boost::asio;             // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;        // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;        // from <boost/asio/ip/tcp.hpp>
using EventCallback = std::function<void(const std::string)>;

class Session : public std::enable_shared_from_this<Session>
{
    ssl::context ctx_;
    tcp::resolver resolver_;
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string port_;
    std::string path_;
    std::string subscribe_request_;
    std::string unsubscribe_request_;
    EventCallback event_callback_;
    // std::string subscription_id_ = "";
    std::promise<bool> subscription_promise_;
    std::promise<bool> subscription_finished_promise_;
    bool subscription_success = false;

public:
    // Resolver and socket require an io_context
    Session(
        const Configuration& configuration,
        const std::string& subscribe_request,
        const std::string& unsubscribe_request,
        const std::string& path,
        net::io_context& ioc,
        const EventCallback event_callback);

    // Start the asynchronous operation
    std::future<bool> start();
    std::future<bool> stop();

private:
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);

    void on_connect(
        beast::error_code ec,
        tcp::resolver::results_type::endpoint_type);

    void on_ssl_handshake(beast::error_code ec);

    void on_handshake(beast::error_code ec);

    void on_write(beast::error_code ec, std::size_t bytes_transferred);

    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    void on_close(beast::error_code ec);
};
