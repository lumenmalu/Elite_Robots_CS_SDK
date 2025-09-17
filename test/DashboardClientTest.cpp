#include <gtest/gtest.h>
#include <string>
#include <thread>
#include "Dashboard/DashboardClient.hpp"

using namespace ELITE;

static std::string s_robot_ip = "192.168.186.140";

class DashboardClientTest : public ::testing::Test {
protected:
    void SetUp() {
        dashboard_client_.reset(new DashboardClient());
    }

    void TearDown() {
        dashboard_client_.reset();
    }

    std::unique_ptr<DashboardClient> dashboard_client_;
};

TEST_F(DashboardClientTest, connect) {
    EXPECT_TRUE(dashboard_client_->connect(s_robot_ip));
    dashboard_client_->closeSafetyDialog();
}

TEST_F(DashboardClientTest, run_program) {
    EXPECT_TRUE(dashboard_client_->connect(s_robot_ip));
    EXPECT_TRUE(dashboard_client_->loadTask("test.task"));
    EXPECT_TRUE(dashboard_client_->powerOff());
    std::this_thread::sleep_for(std::chrono::seconds(5));
    EXPECT_TRUE(dashboard_client_->powerOn());
    EXPECT_TRUE(dashboard_client_->brakeRelease());
    EXPECT_TRUE(dashboard_client_->playProgram());
    EXPECT_TRUE(dashboard_client_->pauseProgram());
    EXPECT_TRUE(dashboard_client_->playProgram());
    EXPECT_TRUE(dashboard_client_->stopProgram());
    EXPECT_TRUE(dashboard_client_->powerOff());
}

TEST_F(DashboardClientTest, load_configuration) {
    EXPECT_TRUE(dashboard_client_->connect(s_robot_ip));
    EXPECT_TRUE(dashboard_client_->loadConfiguration("default.configuration"));
}

TEST_F(DashboardClientTest, log_and_getters)
{
    std::string msg;
    EXPECT_TRUE(dashboard_client_->connect(s_robot_ip));
    EXPECT_TRUE(dashboard_client_->log("Testing Log:"));
    msg = dashboard_client_->version();
    EXPECT_TRUE(!msg.empty());
    EXPECT_TRUE(dashboard_client_->log("Version: " + msg));
    RobotMode mode = dashboard_client_->robotMode();
    EXPECT_TRUE(mode != RobotMode::UNKNOWN);
    EXPECT_TRUE(dashboard_client_->log("Robot mode: " + std::to_string((int)mode)));
    msg = dashboard_client_->getTaskPath();
    EXPECT_TRUE(!msg.empty());
    EXPECT_TRUE(dashboard_client_->log("Loaded program: " + msg));
    EXPECT_TRUE(dashboard_client_->stopProgram());
    TaskStatus status = dashboard_client_->getTaskStatus();
    EXPECT_TRUE(status == TaskStatus::STOPPED);
    EXPECT_TRUE(dashboard_client_->log("Program state: " + std::to_string((int)status)));
}

int main(int argc, char** argv) {
    if(argc >= 2) {
        s_robot_ip = argv[1];
    }
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
