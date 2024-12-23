# Dashboard

## Class
- DashboardClient

## Background
The Dashboard Shell is a way to interact with the robot. By connecting to port 29999 of the robot through TCP, operations such as powering on and off the robot, releasing the brake, and loading and querying tasks can be achieved. The SDK provides the `DashboardClient` class, which contains most of the dashboard interfaces.

## Prerequisites
- Ensure that the SDK is installed correctly.

## Usage

Create a code file:
```bash
touch dashboard_example.cpp
```

Copy the following code and paste it into `dashboard_example.cpp` using your preferred text editor:

```cpp
#include <iostream>

#include <Elite/DashboardClient.hpp>

using namespace ELITE;


const std::string DEFAULT_ROBOT_IP = "192.168.51.244";

int main(int argc, char* argv[]) {
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }

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
    
    my_dashboard->disconnect();

    return 0;
}
```

Compile `dashboard_example.cpp` using the gcc compiler:
```bash
g++ dashboard_example.cpp -o dashboard_example -lelite-cs-series-sdk
```

Run the compiled binary program:
```bash
./dashboard_example <your robot IP>
```

If everything is normal, the following should be output:
```
Connect to robot
Echo right response
```

## Examine the code

At the beginning of the code are the header files of the C++ standard. After the standard header files is `Elite/DashboardClient.hpp`. After including this header file, the dashboard interfaces can be used.
```cpp
#include <iostream>

#include <Elite/DashboardClient.hpp>

using namespace ELITE;
```

The main function is where the program starts. The function of the beginning part of the main function is: if there is a robot IP parameter after running `dashboard_example`, use this IP; otherwise, use the default IP.

```cpp
int main(int argc, char* argv[]) {
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }
```

Immediately after, a smart pointer is created and an instance of `DashboardClient` is created in memory:
```cpp
    std::unique_ptr<DashboardClient> my_dashboard;
    my_dashboard.reset(new DashboardClient());
```

Methods of `DashboardClient` can be called through the `->` symbol. Call the `connect()` method to connect to the robot, and call the `echo()` method to run the `echo` command of the dashboard shell. If the robot responds correctly, this method will return `true`. Output according to the return value. Finally, call the `disconnect()` method to disconnect from the dashboard.

```cpp
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

    my_dashboard->disconnect();
```