#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <memory>
#include <thread>

#include "TrajectoryInterface.hpp"
#include "ControlCommon.hpp"

using namespace ELITE;
using namespace std::chrono;

#define SCRIPT_COMMAND_INTERFACE_TEST_PORT 50004
#define TRAJECTORY_INTERFACE_TEST_PORT 50003


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



TEST(TRAJECTORY_INTERFACE, DISCONNECT) { 
    std::unique_ptr<TrajectoryInterface> trajectory_ins;

    trajectory_ins.reset(new TrajectoryInterface(TRAJECTORY_INTERFACE_TEST_PORT));

    std::unique_ptr<TcpClient> client;
    client.reset(new TcpClient());
    EXPECT_NO_THROW(client->connect("127.0.0.1", TRAJECTORY_INTERFACE_TEST_PORT));

    std::this_thread::sleep_for(50ms);

    EXPECT_TRUE(trajectory_ins->isRobotConnect());

    client.reset();

    std::this_thread::sleep_for(50ms);

    EXPECT_FALSE(trajectory_ins->isRobotConnect());

}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
