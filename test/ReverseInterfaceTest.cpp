#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <memory>
#include <thread>

#include "ReverseInterface.hpp"
#include "ControlCommon.hpp"

#define REVERSE_INTERFACE_TEST_PORT 50002

using namespace ELITE;
using namespace std::chrono;

class TcpClient
{
public:
    boost::asio::io_context io_context;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket_ptr;
    std::unique_ptr<boost::asio::ip::tcp::resolver> resolver_ptr;
    TcpClient() = default;
    
    TcpClient(const std::string& ip, int port) {
        connect(ip, port);
    }

    ~TcpClient() = default;

    void connect(const std::string& ip, int port) {
        try {
            socket_ptr.reset(new boost::asio::ip::tcp::socket(io_context));
            resolver_ptr.reset(new boost::asio::ip::tcp::resolver(io_context));
            socket_ptr->open(boost::asio::ip::tcp::v4());
            boost::asio::ip::tcp::no_delay no_delay_option(true);
            socket_ptr->set_option(no_delay_option);
            boost::asio::socket_base::reuse_address sol_reuse_option(true);
            socket_ptr->set_option(sol_reuse_option);
#if defined(__linux) || defined(linux) || defined(__linux__)
            boost::asio::detail::socket_option::boolean<IPPROTO_TCP, TCP_QUICKACK> quickack(true);
            socket_ptr->set_option(quickack);
#endif
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port);
            socket_ptr->async_connect(endpoint, [&](const boost::system::error_code& error) {
                if (error) {
                    throw boost::system::system_error(error);
                }
            });
            io_context.run();
        
        } catch(const boost::system::system_error &error) {
            throw error;
        }
    }
    
};


TEST(REVERSE_INTERFACE, DISCONNECT) { 
    std::unique_ptr<ReverseInterface> reverse_ins;

    reverse_ins.reset(new ReverseInterface(REVERSE_INTERFACE_TEST_PORT));

    std::unique_ptr<TcpClient> client;
    client.reset(new TcpClient());
    EXPECT_NO_THROW(client->connect("127.0.0.1", REVERSE_INTERFACE_TEST_PORT));

    std::this_thread::sleep_for(50ms);

    EXPECT_TRUE(reverse_ins->isRobotConnect());

    client.reset();

    std::this_thread::sleep_for(50ms);

    EXPECT_FALSE(reverse_ins->isRobotConnect());

}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
