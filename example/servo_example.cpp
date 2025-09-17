﻿#include <Elite/EliteDriver.hpp>
#include <Elite/RtsiClientInterface.hpp>
#include <Elite/DataType.hpp>
#include <Elite/DashboardClient.hpp>

#include <iostream>
#include <memory>
#include <thread>
#include <cmath>

using namespace ELITE;
using namespace std::chrono;
const std::string DEFAULT_ROBOT_IP = "192.168.186.140";
const std::string DEFAULT_LOCAL_IP = "192.168.186.1";

static std::unique_ptr<EliteDriver> s_driver;
static std::unique_ptr<RtsiClientInterface> s_rtsi_client;
static std::unique_ptr<DashboardClient> s_dashboard;

// 定义一个合适的 epsilon 值，这个值可以根据具体情况调整
const double EPSILON = 1e-4;

bool areAlmostEqual(double a, double b) {
    return std::fabs(a - b) < EPSILON;
}


bool areAlmostEqual(const vector6d_t& a, const vector6d_t& b) {
    for (size_t i = 0; i < a.size(); i++) {
        if (!areAlmostEqual(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

template<typename T>
void printArray(T& l) {
    for (auto& i : l) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

void waitRobotArrive(std::vector<std::shared_ptr<RtsiRecipe>> recipe_list, const ELITE::vector6d_t& target) {
    vector6d_t acutal_joint;
    while (true) {
        if(s_rtsi_client->receiveData(recipe_list) > 0) {
            if(!recipe_list[0]->getValue("actual_joint_positions", acutal_joint)) {
                std::cout << "Recipe has wrong" << std::endl;
                continue;
            }
            if (areAlmostEqual(acutal_joint, target)) {
                break;
            } else {
                if(!s_driver->writeServoj(target, 100)) {
                    return;
                }
            }
        } else {
            std::cout << "Couldn't receive data" << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    // if (argc < 3) {
    //     std::cout << "Must provide robot ip or local ip. Command like: ./servo_example 192.168.1.250 192.168.1.251" << std::endl;
    //     return 1;
    // }
    s_driver = std::make_unique<EliteDriver>(DEFAULT_ROBOT_IP, DEFAULT_LOCAL_IP, "external_control.script");
    s_rtsi_client = std::make_unique<RtsiClientInterface>();
    s_dashboard = std::make_unique<DashboardClient>();
    
    if (!s_dashboard->connect(DEFAULT_ROBOT_IP)) {
        return 1;
    }
    std::cout << "Dashboard connected" << std::endl;

    s_rtsi_client->connect(DEFAULT_ROBOT_IP);
    std::cout << "RTSI connected" << std::endl;

    if (!s_rtsi_client->negotiateProtocolVersion()) {
        return 1;
    }
    
    std::cout << "Controller version is " << s_rtsi_client->getControllerVersion().toString() << std::endl;

    auto recipe = s_rtsi_client->setupOutputRecipe({"actual_joint_positions"}, 125);
    if (!recipe) {
        return 1;
    }
    
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
        std::this_thread::sleep_for(10ms);
    }
    
    // wait robot
    std::this_thread::sleep_for(1s);

    if (!s_rtsi_client->start()) {
        return 1;
    }
    std::vector<std::shared_ptr<RtsiRecipe>> recipe_list{recipe};
    vector6d_t acutal_joint;
    
    if(s_rtsi_client->receiveData(recipe_list) <= 0) {
        std::cout << "RTSI recipe receive none" << std::endl;
        return 1;
    }

    if(!recipe_list[0]->getValue("actual_joint_positions", acutal_joint)) {
        std::cout << "Recipe has wrong" << std::endl;
        return 1;
    }

    if(!s_rtsi_client->pause()) {
        return 1;
    }

    // Make target points
    std::vector<vector6d_t> target_positions_1;
    vector6d_t target_joint = acutal_joint;
    
    if (acutal_joint[5] <= 3) {
        for (double target = acutal_joint[5]; target < 3; target += 0.001) {
            target_joint[5] = target;
            target_positions_1.push_back(target_joint);
        }
    } else {
        for (double target = acutal_joint[5]; target > 3; target -= 0.001) {
            target_joint[5] = target;
            target_positions_1.push_back(target_joint);
        }
    }

    std::vector<vector6d_t> target_positions_2;
    for (double target = (*(target_positions_1.end() - 1))[5]; target > -3; target -= 0.002) {
        target_joint[5] = target;
        target_positions_2.push_back(target_joint);
    }

    for (auto& target : target_positions_1) {
        if(!s_driver->writeServoj(target, 100)) {
            return 1;
        }
        std::this_thread::sleep_for(4ms);
    }
    
    if (!s_rtsi_client->start()) {
        return 1;
    }

    waitRobotArrive(recipe_list, *(target_positions_1.end() - 1));

    if(!s_rtsi_client->pause()) {
        return 1;
    }

    for (auto& target : target_positions_2) {
        if(!s_driver->writeServoj(target, 100)) {
            return 1;
        }
        std::this_thread::sleep_for(4ms);
    }
    
    if (!s_rtsi_client->start()) {
        return 1;
    }

    waitRobotArrive(recipe_list, *(target_positions_2.end() - 1));

    s_driver->stopControl();

    return 0;
}
