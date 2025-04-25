#include "Primary/PrimaryPort.hpp"
#include "Primary/RobotConfPackage.hpp"

#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <thread>

using namespace ELITE;
using namespace std::chrono;

static std::string s_robot_ip = "192.168.51.127";

TEST(PrimaryPortTest, multiple_connect) {
    std::unique_ptr<PrimaryPort> primary =  std::make_unique<PrimaryPort>();

    for (size_t i = 0; i < 100; i++) {
        EXPECT_TRUE(primary->connect(s_robot_ip, 30001));
        std::cout << "Connect count: " << i << std::endl;
        std::this_thread::sleep_for(10ms);
    }

    std::shared_ptr<KinematicsInfo> ki = std::make_shared<KinematicsInfo>();
    EXPECT_TRUE(primary->getPackage(ki, 500));
    
    primary->disconnect();
}

TEST(PrimaryPortTest, get_package) {
    std::unique_ptr<PrimaryPort> primary =  std::make_unique<PrimaryPort>();

    EXPECT_TRUE(primary->connect(s_robot_ip, 30001));

    std::shared_ptr<KinematicsInfo> ki = std::make_shared<KinematicsInfo>();
    // Will cost 10 second at least
    for (size_t i = 0; i < 100; i++) {
        EXPECT_TRUE(primary->getPackage(ki, 500));
    }
    for (size_t i = 0; i < 6; i++) {
        std::cout << "DH A " << i <<": " << ki->dh_a_[i] << std::endl;
        std::cout << "DH D " << i <<": " << ki->dh_d_[i] << std::endl;
        std::cout << "DH ALPHA " << i <<": " << ki->dh_alpha_[i] << std::endl;
    }
    primary->disconnect();
}


int main(int argc, char** argv) {
    if(argc >= 2) {
        s_robot_ip = argv[1];
    }
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
