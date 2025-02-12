#include "ScriptSender.hpp"
#include "ControlCommon.hpp"
#include "EliteException.hpp"
#include "Log.hpp"
#include <boost/asio.hpp>

using namespace ELITE;



ScriptSender::ScriptSender(int port, const std::string& program) : program_(program) {
    server_.reset(new TcpServer(port));
    server_->setConnectCallback([&](std::shared_ptr<boost::asio::ip::tcp::socket> client){ 
        if (client_ && client->is_open()) {
            ELITE_LOG_INFO("Script sender has new connection, previous connection will be dropped.");
            server_->releaseClient(client_);
        }
        client_ = client;
        ELITE_LOG_INFO("Script sender accept new connection.");
        responseRequest();
    });
}


ScriptSender::~ScriptSender() {
    
}


void ScriptSender::responseRequest() {
    if (!client_) {
        return;
    }
    boost::asio::async_read_until(
        *client_,
        recv_request_buffer_,
        '\n',
        [&](boost::system::error_code ec, std::size_t len) {
            if (ec || len <= 0) {
                if (client_->is_open()) {
                    ELITE_LOG_INFO("Connection to script sender interface dropped: %s", boost::system::system_error(ec).what());
                    server_->releaseClient(client_);
                }
                return;
            }
            ELITE_LOG_INFO("Robot request external control script.");
            std::string request;
            std::istream response_stream(&recv_request_buffer_);
            std::getline(response_stream, request);
            if (request == PROGRAM_REQUEST_) {
                boost::system::error_code wec;
                client_->write_some(boost::asio::buffer(program_), wec);
                if (wec) {
                    ELITE_LOG_ERROR("Script sender send script fail: %s", boost::system::system_error(wec).what());
                    return;
                }
            }
            responseRequest();
        }
    );
}
