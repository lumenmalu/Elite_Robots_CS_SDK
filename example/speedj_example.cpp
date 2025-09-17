#include <Elite/EliteDriver.hpp>
#include <Elite/RtsiIOInterface.hpp>
#include <Elite/DataType.hpp>
#include <Elite/DashboardClient.hpp>

#include <iostream>
#include <memory>

using namespace ELITE;

static std::unique_ptr<EliteDriver> s_driver;
static std::unique_ptr<DashboardClient> s_dashboard;

const std::string DEFAULT_ROBOT_IP = "192.168.186.140";
const std::string DEFAULT_LOCAL_IP = "192.168.186.1";

int main(int argc, char** argv) {
    // if (argc < 3) {
    //     std::cout << "Must provide robot ip or local ip. Command like: ./speedj_example 192.168.1.250 192.168.1.251" << std::endl;
    //     return 1;
    // }
    s_driver = std::make_unique<EliteDriver>(DEFAULT_ROBOT_IP, DEFAULT_LOCAL_IP, "external_control.script");
    s_dashboard = std::make_unique<DashboardClient>();
    
    if (!s_dashboard->connect(DEFAULT_ROBOT_IP)) {
        return 1;
    }
    std::cout << "Dashboard connected" << std::endl;

    if(!s_dashboard->powerOn()) {
        return 1;
    }
    std::cout << "Robot power on" << std::endl;

    if (!s_dashboard->brakeRelease()) {
        return 1;
    }
    std::cout << "Robot brake released" << std::endl;

    if (!s_dashboard->playProgram()) {
        return 1;
    }
    std::cout << "Program run" << std::endl;

    while (!s_driver->isRobotConnected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    vector6d_t speedl_vector{0, 0, 0, 0, 0, -0.02};
    while (true) {
        speedl_vector = {0, 0, 0, 0, 0, -0.02};
        s_driver->writeSpeedj(speedl_vector, 0);

        std::this_thread::sleep_for(std::chrono::seconds(3));

        speedl_vector = {0, 0, 0, 0, 0, 0.02};
        s_driver->writeSpeedj(speedl_vector, 0);
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    return 0;
}
