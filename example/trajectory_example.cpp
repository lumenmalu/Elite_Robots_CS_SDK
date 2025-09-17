#include <Elite/EliteDriver.hpp>
#include <Elite/RtsiIOInterface.hpp>
#include <Elite/DataType.hpp>
#include <Elite/DashboardClient.hpp>

#include <memory>
#include <thread>
#include <iostream>

using namespace ELITE;

static std::unique_ptr<EliteDriver> s_driver;
static std::unique_ptr<RtsiIOInterface> s_rtsi_client;
static std::unique_ptr<DashboardClient> s_dashboard;

const std::string DEFAULT_ROBOT_IP = "192.168.186.140";
const std::string DEFAULT_LOCAL_IP = "192.168.186.1";

int main(int argc, const char** argv) {
    // if (argc < 3) {
    //     std::cout << "Must provide robot ip or local ip. Command like: ./trajectory_example 192.168.1.250 192.168.1.251" << std::endl;
    //     return 1;
    // }
    s_driver = std::make_unique<EliteDriver>(DEFAULT_ROBOT_IP, DEFAULT_LOCAL_IP, "external_control.script");
    s_rtsi_client = std::make_unique<RtsiIOInterface>("output_recipe.txt", "input_recipe.txt", 250);
    s_dashboard = std::make_unique<DashboardClient>();
    
    if (!s_dashboard->connect(DEFAULT_ROBOT_IP)) {
        return 1;
    }
    std::cout << "Dashboard connected" << std::endl;

    s_rtsi_client->connect(DEFAULT_ROBOT_IP);
    std::cout << "RTSI connected" << std::endl;

    bool is_move_finish = false;
    s_driver->setTrajectoryResultCallback([&](TrajectoryMotionResult result){
        if (result == TrajectoryMotionResult::SUCCESS) {
            is_move_finish = true;
        }
    });

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
    
    vector6d_t actual_joints = s_rtsi_client->getActualJointPositions();

    s_driver->writeTrajectoryControlAction(ELITE::TrajectoryControlAction::START, 1, 200);

    actual_joints[3] = -1.57;
    s_driver->writeTrajectoryPoint(actual_joints, 3, 0, false);

    while (!is_move_finish) {
        s_driver->writeTrajectoryControlAction(ELITE::TrajectoryControlAction::NOOP, 0, 200);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Joints moved to target" << std::endl;

    vector6d_t actual_pose = s_rtsi_client->getAcutalTCPPose();

    is_move_finish = false;

    s_driver->writeTrajectoryControlAction(ELITE::TrajectoryControlAction::START, 3, 200);
    
    actual_pose[2] -= 0.2;
    s_driver->writeTrajectoryPoint(actual_pose, 3, 0.05, true);

    actual_pose[1] -= 0.2;
    s_driver->writeTrajectoryPoint(actual_pose, 3, 0.05, true);

    actual_pose[1] += 0.2;
    actual_pose[2] += 0.2;
    s_driver->writeTrajectoryPoint(actual_pose, 3, 0.05, true);

    while (!is_move_finish) {
        s_driver->writeTrajectoryControlAction(ELITE::TrajectoryControlAction::NOOP, 0, 200);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Line moved to target" << std::endl;

    s_driver->writeTrajectoryControlAction(ELITE::TrajectoryControlAction::CANCEL, 1, 200);
    s_driver->stopControl();
    return 0;
}
