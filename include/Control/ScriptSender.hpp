#ifndef __SCRIPT_SENDER_HPP__
#define __SCRIPT_SENDER_HPP__

#include "TcpServer.hpp"

#include <boost/asio.hpp>
#include <string>
#include <memory>

namespace ELITE
{

class ScriptSender {
private:
    const std::string PROGRAM_REQUEST_ = std::string("request_program");
    std::unique_ptr<TcpServer> server_;
    const std::string& program_;
    std::shared_ptr<boost::asio::ip::tcp::socket> client_;
    boost::asio::streambuf recv_request_buffer_;

    void responseRequest();

public:
    ScriptSender(int port, const std::string& program);
    ~ScriptSender();
};








} // namespace ELITE






#endif
