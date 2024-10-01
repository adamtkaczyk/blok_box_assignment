#include "session.hpp"

#include <boost/core/ignore_unused.hpp>
#include <boost/system/error_code.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <exception>
#include <sstream>

Session::Session(
    const Configuration& configuration,
    const std::string& subscribe_request,
    const std::string& unsubscribe_request,
    const std::string& path,
    net::io_context& ioc,
    const EventCallback event_callback)
    : ctx_([&configuration]() {
        ssl::context ctx{ssl::context::tlsv12_client};
        return ctx;
    }())
    , resolver_(net::make_strand(ioc))
    , ws_(net::make_strand(ioc), ctx_)
    , host_(configuration.host)
    , port_(configuration.port)
    , path_(path)
    , subscribe_request_(subscribe_request)
    , unsubscribe_request_(unsubscribe_request)
    , event_callback_(event_callback)
{
}

// Start the asynchronous operation
std::future<bool> Session::start()
{
    // Look up the domain name
    resolver_.async_resolve(
        host_.c_str(),
        port_.c_str(),
        beast::bind_front_handler(&Session::on_resolve, shared_from_this()));

    return subscription_promise_.get_future();
}

void Session::on_resolve(
    beast::error_code ec,
    tcp::resolver::results_type results)
{
    if (ec) {
        printf("Event session error: resolve: %s\n", ec.message().c_str());
        return;
    }

    // Set a timeout on the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(ws_).async_connect(
        results,
        beast::bind_front_handler(&Session::on_connect, shared_from_this()));
}

void Session::on_connect(
    beast::error_code ec,
    tcp::resolver::results_type::endpoint_type)
{
    if (ec) {
        printf("Event session error: connect: %s\n", ec.message().c_str());
        return;
    }

    // Set a timeout on the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    ws_.next_layer().async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(
            &Session::on_ssl_handshake, shared_from_this()));
}

void Session::on_ssl_handshake(beast::error_code ec)
{
    if (ec) {
        printf("Event session error: ssl_handshake: %s\n", ec.message().c_str());
        return;
    }

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::client));

    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(
        websocket::stream_base::decorator([](websocket::request_type& req) {
            req.set(
                http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-async");
        }));

    // Perform the websocket handshake
    ws_.async_handshake(
        host_,
        path_,
        beast::bind_front_handler(&Session::on_handshake, shared_from_this()));
}

void Session::on_handshake(beast::error_code ec)
{
    if (ec) {
        printf("Event session error: handshake: %s\n", ec.message().c_str());
        return;
    }

    // Send the message
    printf("Send subscribe_request: %s\n", subscribe_request_.c_str());
    ws_.async_write(
        net::buffer(subscribe_request_),
        beast::bind_front_handler(&Session::on_write, shared_from_this()));
}

void Session::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        printf("Event session error: handshake: %s\n", ec.message().c_str());
        return;
    }

    // Read a message into our buffer
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(&Session::on_read, shared_from_this()));
}

void Session::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        printf("Event session error: read: %s\n", ec.message().c_str());
        return;
    }

    try {
        auto response = boost::beast::buffers_to_string(buffer_.data());

        auto json_response = nlohmann::json::parse(response);

        if (json_response.contains("result") && json_response.contains("id") && json_response["id"].is_number()) {
            auto id = json_response["id"].get<int>();
            if (!subscription_success) {
                printf("Subscription request send successfull for id: %d\n", id);
                subscription_success = true;
                subscription_promise_.set_value(true);
            } else {
                printf("Unsubscription request send successfull for id: %d\n", id);
                // Close the WebSocket connection
                ws_.async_close(
                    websocket::close_code::normal,
                    beast::bind_front_handler(&Session::on_close, shared_from_this()));
                return;
            }
        }

        event_callback_(response);
    } catch (const std::exception& e) {
        printf("Event json parsing exception: %s\n", e.what());
        printf("Buffer: %s\n", boost::beast::buffers_to_string(buffer_.data()).c_str());
    }
    buffer_.clear();
    // Read a message into our buffer
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(&Session::on_read, shared_from_this()));
}

std::future<bool> Session::stop()
{
        printf("Send unsubscribe_request:\n");
        // Send the message
        ws_.write(net::buffer(unsubscribe_request_));


    return subscription_finished_promise_.get_future();
}

void Session::on_close(beast::error_code ec)
{
    printf("Close event subscription session\n");
    if (ec) {
        printf("Error: close: %s\n", ec.message().c_str());
        return;
    }
    subscription_finished_promise_.set_value(true);
}
