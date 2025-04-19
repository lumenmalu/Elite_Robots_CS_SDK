#include "PrimaryPort.hpp"
#include "EliteException.hpp"
#include "Utils.hpp"
#include "Log.hpp"

namespace ELITE
{
using namespace std::chrono;

PrimaryPort::PrimaryPort() {
    message_head_.resize(HEAD_LENGTH);
}

PrimaryPort::~PrimaryPort() {
    disconnect();
}


bool PrimaryPort::connect(const std::string& ip, int port) {
    try {
        std::lock_guard<std::mutex> lock(socket_mutex_);
        socket_ptr_.reset(new boost::asio::ip::tcp::socket(io_context_));
        resolver_ptr_.reset(new boost::asio::ip::tcp::resolver(io_context_));
        socket_ptr_->open(boost::asio::ip::tcp::v4());
        socket_ptr_->set_option(boost::asio::ip::tcp::no_delay(true));
        socket_ptr_->set_option(boost::asio::socket_base::reuse_address(true));
        socket_ptr_->set_option(boost::asio::socket_base::keep_alive(false));
#if defined(__linux) || defined(linux) || defined(__linux__)
        socket_ptr_->set_option(boost::asio::detail::socket_option::boolean<IPPROTO_TCP, TCP_QUICKACK>(true));
#endif
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(ip), port);
        boost::system::error_code connect_ec;
        socket_ptr_->async_connect(endpoint, [&](const boost::system::error_code& ec){
            connect_ec = ec;
        });
        if (io_context_.stopped()) {
            io_context_.restart();
        }
        io_context_.run_for(std::chrono::steady_clock::duration(500ms));
        if (connect_ec) {
            ELITE_LOG_ERROR("Connect to robot primary port fail: %s", boost::system::system_error(connect_ec).what());
            return false;
        }
    } catch(const boost::system::system_error &error) {
        throw EliteException(EliteException::Code::SOCKET_CONNECT_FAIL, error.what());
        return false;
    }

    // Added asynchronously read and parse packets
    parserMessageHead();
    // Start async thread
    socket_async_thread_alive_ = true;
    socket_async_thread_.reset(new std::thread([&](){
        socketAsyncLoop();
    }));
    
    return true;
}

void PrimaryPort::socketClose() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    socket_async_thread_alive_ = false;
    if (socket_ptr_) {
        boost::system::error_code ec;
        socket_ptr_->cancel(ec);
        socket_ptr_->close(ec);
        if (ec) {
            throw EliteException(EliteException::Code::SOCKET_FAIL, 
                                 boost::system::system_error(ec).what());
            return;
        }
    }
    socket_ptr_.reset();
}

void PrimaryPort::disconnect() {
    socketClose();
    io_context_.stop();
    if (socket_async_thread_ && socket_async_thread_->joinable()) {
        socket_async_thread_->join();
    }
    socket_async_thread_.reset();
}

bool PrimaryPort::sendScript(const std::string& script) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (!socket_ptr_) {
        ELITE_LOG_ERROR("Don't connect to robot primary port");
        return false;
    }
    auto script_with_newline = std::make_shared<std::string>(script + "\n");
    boost::system::error_code ec;
    socket_ptr_->write_some(boost::asio::buffer(*script_with_newline), ec);
    if (ec) {
        ELITE_LOG_ERROR("Send script to robot fail : ", boost::system::system_error(ec).what());
        return false;
    } else {
        return true;
    }
}

bool PrimaryPort::getPackage(std::shared_ptr<PrimaryPackage> pkg, int timeout_ms) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        parser_sub_msg_.insert({pkg->getType(), pkg});
    }

    return pkg->waitUpdate(timeout_ms);
}

void PrimaryPort::parserMessageHead() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (!socket_ptr_) {
        return;
    }

    auto head_func = 
        [&](const boost::system::error_code &ec, std::size_t nb) {
            if (!ec && nb == HEAD_LENGTH) {
                uint32_t len = 0;
                UTILS::EndianUtils::unpack(message_head_.begin(), len);
                if (len <= HEAD_LENGTH) {
                    throw EliteException(EliteException::Code::SOCKET_FAIL);
                }
                parserMessageBody(message_head_[4], len);
            } else {
                throw EliteException(EliteException::Code::SOCKET_FAIL, ec.message());
            }
    };

    boost::asio::async_read(
        *socket_ptr_,
        boost::asio::buffer(message_head_),
        head_func
    );
}

void PrimaryPort::parserMessageBody(int type, int len) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (!socket_ptr_) {
        return;
    }

    int body_len = len - HEAD_LENGTH;
    message_body_.resize(body_len);
    // If not RobotState message consume it with no parser.
    if (type != ROBOT_STATE_MSG_TYPE) {
        boost::asio::async_read(
            *socket_ptr_, 
            boost::asio::buffer(message_body_, body_len), 
            [&](const boost::system::error_code &ec, std::size_t nb) {
                parserMessageHead();
            }
        );
    } else {
        boost::asio::async_read(
            *socket_ptr_, 
            boost::asio::buffer(message_body_, body_len), 
            [&, body_len](const boost::system::error_code &ec, std::size_t nb) {
                if (nb != body_len) {
                    throw EliteException(EliteException::Code::SOCKET_FAIL);
                }
                uint32_t sub_len = 0;
                for (auto iter = message_body_.begin(); iter < message_body_.end(); iter += sub_len) {
                    UTILS::EndianUtils::unpack(iter, sub_len);
                    int sub_type = *(iter + 4);

                    std::lock_guard<std::mutex> lock(mutex_);
                    auto psm = parser_sub_msg_.find(sub_type);
                    if (psm != parser_sub_msg_.end()) {
                        psm->second->parser(sub_len, iter);
                        psm->second->notifyUpated();
                        parser_sub_msg_.erase(sub_type);
                    }
                }
                parserMessageHead();
            }
        );
    }
}

void PrimaryPort::socketAsyncLoop() {
    while (socket_async_thread_alive_) {
        try {
            if (io_context_.stopped()) {
                auto work = boost::asio::make_work_guard(io_context_);
                io_context_.run();
                work.reset();
            }
            io_context_.run();
        } catch(const std::exception& e) {
            socket_async_thread_alive_ = false;
        }
    }
}


}