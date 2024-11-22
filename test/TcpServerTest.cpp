#include <gtest/gtest.h>
#include <string>
#include <thread>
#include "Common/TcpServer.hpp"
#include "boost/asio.hpp"
#include <iostream>

using namespace std::chrono;

#define SERVER_TEST_PORT (50001)

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



TEST(TCP_SERVER, TCP_SERVER_TEST) {  
    ELITE::TcpServer server(SERVER_TEST_PORT);
    static uint8_t read_buff[4096];
    int send_data = 1234;

    std::shared_ptr<boost::asio::ip::tcp::socket> client_socket;
    server.setConnectCallback([&](std::shared_ptr<boost::asio::ip::tcp::socket> client) {
        client_socket = client;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    TcpClient client("127.0.0.1", SERVER_TEST_PORT);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(client_socket != nullptr);
    if (client_socket == nullptr) {
        return;
    }

    client_socket->async_read_some(boost::asio::buffer(read_buff, sizeof(read_buff)), [&](const boost::system::error_code &ec, std::size_t nb) {
        if (ec) {
            ASSERT_TRUE(false);
        } else {
            ASSERT_EQ(*(int*)read_buff, send_data);
        }
    });

    client.socket_ptr->send(boost::asio::buffer(&send_data, sizeof(send_data)));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    client.socket_ptr->close();
    
    client_socket->async_read_some(boost::asio::buffer(read_buff, sizeof(read_buff)), [&](const boost::system::error_code &ec, std::size_t nb) {
        if (!ec) {
            ASSERT_TRUE(false);
        }
        client_socket.reset();
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    ASSERT_FALSE(client_socket);
}

TEST(TCP_SERVER, TCP_SERVER_MULIT_CONNECT) {
    ELITE::TcpServer server(SERVER_TEST_PORT);
    TcpClient client1;
    TcpClient client2;
    const std::string client_send_string = "client_send_string\n";

    std::shared_ptr<boost::asio::ip::tcp::socket> server_client_socket;
    
    server.setConnectCallback([&](std::shared_ptr<boost::asio::ip::tcp::socket> client) {
        if (server_client_socket) {
            server_client_socket.reset();
        }
        server_client_socket = client;
    });

    // Wait for TCPServer thread run
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    client1.connect("127.0.0.1", SERVER_TEST_PORT);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_TRUE(server_client_socket != nullptr);
    if (server_client_socket == nullptr) {
        return;
    }
    boost::asio::ip::tcp::socket* server_client_socket_client1 = server_client_socket.get();
    
    std::string recv_buffer;
    int read_len = 0;
    boost::asio::async_read_until(
        *server_client_socket, 
        boost::asio::dynamic_buffer(recv_buffer), 
        '\n', 
        [&](boost::system::error_code ec, std::size_t len){
            if(!ec) {
                read_len += len;
            }
        }
    );
    ASSERT_EQ(client1.socket_ptr->write_some(boost::asio::buffer(client_send_string)), client_send_string.length());
    // Wait for recv
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(read_len, client_send_string.length());
    ASSERT_EQ(recv_buffer.length(), client_send_string.length());
    ASSERT_EQ(recv_buffer, client_send_string);

    recv_buffer.clear();

    // Add new task
    boost::asio::async_read_until(
        *server_client_socket, 
        boost::asio::dynamic_buffer(recv_buffer), 
        '\n', 
        [&](boost::system::error_code ec, std::size_t len){
            if (!ec) {
                // When new connection come in, old will be shutdown. So should't execute fllow command.
                ASSERT_TRUE(false);
            }
        }
    );
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // New connection
    client2.connect("127.0.0.1", SERVER_TEST_PORT);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_TRUE(server_client_socket != nullptr);
    ASSERT_TRUE(server_client_socket.get() != server_client_socket_client1);
    read_len = 0;
    boost::asio::async_read_until(
        *server_client_socket, 
        boost::asio::dynamic_buffer(recv_buffer), 
        '\n', 
        [&](boost::system::error_code ec, std::size_t len){
            if(!ec) {
                read_len += len;
            }
        }
    );
    ASSERT_EQ(client2.socket_ptr->write_some(boost::asio::buffer(client_send_string)), client_send_string.length());
    // Wait for recv
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(read_len, client_send_string.length());
    ASSERT_EQ(recv_buffer.length(), client_send_string.length());
    ASSERT_EQ(recv_buffer, client_send_string);

    client1.socket_ptr->close();
    client2.socket_ptr->close();
}


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}