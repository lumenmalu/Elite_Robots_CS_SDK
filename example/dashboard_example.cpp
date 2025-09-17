#include "Elite/DashboardClient.hpp"
#include "Elite/DataType.hpp"

#include <thread>
#include <regex>
#include <iostream>

using namespace ELITE;

// In a real-world example it would be better to get those values from command line parameters / a
// better configuration system such as Boost.Program_options
const std::string DEFAULT_ROBOT_IP = "192.168.186.140";

int main(int argc, char* argv[]) {
    // Parse the ip arguments if given
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }

    // Making the robot ready for the program by:
    // Connect the the robot Dashboard
    std::unique_ptr<DashboardClient> my_dashboard;
    my_dashboard.reset(new DashboardClient());
    if (!my_dashboard->connect(robot_ip)) {
        std::cout << "Could not connect to robot" << std::endl;
        return 1;
    } else {
        std::cout << "Connect to robot" << std::endl;
    }

    if (!my_dashboard->echo()) {
        std::cout << "Echo not right response" << std::endl;
        return 1;
    } else {
        std::cout << "Echo right response" << std::endl;
    }
    

    if (!my_dashboard->powerOff())    {
        std::cout << "Could not send power off" << std::endl;
        return 1;
    } else {
        std::cout << "Power off" << std::endl;
    }

    my_dashboard->closeSafetyDialog();

    // Power it on
    if (!my_dashboard->powerOn()) {
        std::cout << "Could not send Power on command" << std::endl;
        return 1;
    } else {
        std::cout << "Power on" << std::endl;
    }

    // Release the brakes
    if (!my_dashboard->brakeRelease()) {
        std::cout << "Could not send BrakeRelease command" << std::endl;
        return 1;
    } else {
        std::cout << "Brake release" << std::endl;
    }

    // Load existing task
    const std::string task_file_name_to_be_loaded("test.task");
    if (!my_dashboard->loadTask(task_file_name_to_be_loaded)) {
        std::cout << "Could not load  " << task_file_name_to_be_loaded.c_str() << std::endl;
        return 1;
    }
    std::string task = my_dashboard->getTaskPath();
    if (task != task_file_name_to_be_loaded) {
        std::cout << "Not load right task" << std::endl;
        return 1;
    } else {
        std::cout << "Load task" << std::endl;
    }

    if (my_dashboard->getTaskStatus() != TaskStatus::STOPPED) {
        std::cout << "Task not stopped" << std::endl;
        return 1;
    } else {
        std::cout << "Task stopped" << std::endl;
    }

    if (!my_dashboard->playProgram()) {
        std::cout << "Could not play task" << std::endl;
        return 1;
    } else {
        std::cout << "Play task" << std::endl;
    }

    if (my_dashboard->getTaskStatus() != TaskStatus::PLAYING) {
        std::cout << "Task not running" << std::endl;
        return 1;
    } else {
        std::cout << "Task running" << std::endl;
    }

    if (!my_dashboard->pauseProgram()) {
        std::cout << "Could not pause task" << std::endl;
        return 1;
    } else {
        std::cout << "Pause task" << std::endl;
    }

    if (my_dashboard->getTaskStatus() != TaskStatus::PAUSED) {
        std::cout << "Task not pause" << std::endl;
        return 1;
    } else {
        std::cout << "Task pause" << std::endl;
    }

    if (!my_dashboard->stopProgram()) {
        std::cout << "Could not stop task" << std::endl;
        return 1;
    } else {
        std::cout << "Stop task" << std::endl;
    }

    if (my_dashboard->getTaskStatus() != TaskStatus::STOPPED) {
        std::cout << "Task not stop" << std::endl;
        return 1;
    } else {
        std::cout << "Task stopped" << std::endl;
    }
    
    
    if (!my_dashboard->isTaskSaved()) {
        std::cout << "Task save status not right" << std::endl;
        return 1;
    } else {
        std::cout << "Task saved" << std::endl;
    }
    
    my_dashboard->disconnect();

    return 0;
}
