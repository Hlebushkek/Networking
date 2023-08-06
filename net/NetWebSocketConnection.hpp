#pragma once

#include "NetCommon.hpp"
#include <boost/beast.hpp>

class WebSocketConnection : public std::enable_shared_from_this<WebSocketConnection> {
public:
    WebSocketConnection(boost::asio::ip::tcp::socket socket)
        : websocket_(std::move(socket)) {}

    void start() {
        websocket_.async_accept(
            [self = shared_from_this()](boost::beast::error_code ec)
        {
            if (!ec)
                self->read();
        });
    }

    void read() {
        websocket_.async_read(buffer_,
            [self = shared_from_this()](boost::beast::error_code ec, std::size_t bytes_transferred)
        {
            if (!ec) {
                // Process the received WebSocket message in 'buffer_'
                
                // After processing, you can optionally echo the message back to the client:
                self->websocket_.text();
                self->websocket_.async_write(self->buffer_.data(),
                    [self](boost::beast::error_code ec, std::size_t /*bytes_transferred*/)
                {
                    if (!ec)
                        self->read();
                });
            }
        });
    }

private:
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> websocket_;
    boost::beast::flat_buffer buffer_;
};