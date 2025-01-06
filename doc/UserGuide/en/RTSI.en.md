# RTSI

## Class
- RtsiClientInterface: Provides the basic operation interface for RTSI.
- RtsiRecipe: Provides the operation interface for RTSI recipes.
- RtsiIOInterface: Encapsulates RTSI subscription items.

## Background
RTSI is a communication interface for Elite CS series robots. It can read and write robot IOs at a frequency of up to 250Hz. The SDK provides two interfaces. The first interface is the basic interface of RTSI and can control every step of RTSI communication. The second interface encapsulates various robot data, such as joint angles, into functions that can be called directly.  
For the specific description of the RTSI protocol, you can download the RTSI description document from the official website.

## Prerequisites
- Ensure that the SDK is installed correctly.

## RtsiClientInterface Usage

Create a code file:
```bash
touch rtsi_base_example.cpp
```

Copy the following code and paste it into `rtsi_base_example.cpp` using your preferred text editor:

```cpp
#include <iostream>
#include <Elite/RtsiClientInterface.hpp>
#include <Elite/RtsiRecipe.hpp>

using namespace ELITE;

const std::string DEFAULT_ROBOT_IP = "192.168.51.244";

int main(int argc, char* argv[]) {
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }

    std::unique_ptr<RtsiClientInterface> rtsi = std::make_unique<RtsiClientInterface>();

    rtsi->connect(robot_ip);

    if(rtsi->negotiateProtocolVersion()) {
        std::cout << "Negotiate protocol version success" << std::endl;
    } else {
        std::cout << "Negotiate protocol version fail" << std::endl;
        return 1;
    }
    

    std::cout << "Controller version: " << rtsi->getControllerVersion().toString() << std::endl;

    auto out_recipe = rtsi->setupOutputRecipe({"timestamp", "actual_joint_positions"}, 250);

    auto in_recipe = rtsi->setupInputRecipe({"speed_slider_mask", "speed_slider_fraction"});

    if(rtsi->start()) {
        std::cout << "RTSI sync start successful" << std::endl;
    } else {
        std::cout << "RTSI sync start fail" << std::endl;
        return 1;
    }
    
    double timestamp;
    vector6d_t actula_joints;
    int count = 250;
    auto out_recipe_list = std::vector<RtsiRecipeSharedPtr>();
    out_recipe_list.push_back(out_recipe);
    while(count--) {
        rtsi->receiveData(out_recipe_list);
        out_recipe->getValue("timestamp", timestamp);
        out_recipe->getValue("actual_joint_positions", actula_joints);
        std::cout << "timestamp: " << timestamp << std::endl;
        std::cout << "actual_joint_positions: ";
        for(auto i : actula_joints) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }

    in_recipe->setValue("speed_slider_mask", 1);
    in_recipe->setValue("speed_slider_fraction", 0.5);
    rtsi->send(in_recipe);

    rtsi->disconnect();

    return 0;
}
```

Compile `rtsi_base_example.cpp` using the gcc compiler:
```bash
g++ rtsi_base_example.cpp -o rtsi_base_example -lelite-cs-series-sdk
```

Run the compiled binary program:
```bash
./rtsi_base_example <your robot IP>
```

If everything is normal, the timestamp and the actual robot joint angles should be output.
> Note: If the program throws an exception `rtsi unknow variable type: variable "speed_slider_mask" error type: IN_USE`, it is recommended to check if there are other RTSI connections or bus connections.

## Examine the code
At the beginning of the code, there are header files for the C++ standard. After the standard header files is `Elite/RtsiClientInterface.hpp`, and after including this header file, the basic interface of RTSI can be used. After that is `Elite/RtsiRecipe.hpp`, which provides the recipe operation methods for RTSI.
```cpp
#include <iostream>
#include <Elite/RtsiClientInterface.hpp>
#include <Elite/RtsiRecipe.hpp>

using namespace ELITE;
```

The main function is where the program starts. The function of the beginning part of the main function is: if there is a robot IP parameter after running `rtsi_base_example`, use this IP; otherwise, use the default IP.
```cpp
int main(int argc, char* argv[]) {
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }
```

Immediately after, a smart pointer is created and an instance of `RtsiClientInterface` is created in memory:
```cpp
    std::unique_ptr<RtsiClientInterface> rtsi = std::make_unique<RtsiClientInterface>();
```

Methods of `RtsiClientInterface` can be called through the `->` symbol. Call the `connect()` method to connect to the robot:
```cpp
    rtsi->connect(robot_ip);
```

The following code is to verify the RTSI protocol version. This step must be performed before communication starts; otherwise, subsequent communication cannot be performed:
```cpp
    if(rtsi->negotiateProtocolVersion()) {
        std::cout << "Negotiate protocol version success" << std::endl;
    } else {
        std::cout << "Negotiate protocol version fail" << std::endl;
        return 1;
    }
```

Use the `getControllerVersion()` method to obtain the controller version and print it.
```cpp
    std::cout << "Controller version: " << rtsi->getControllerVersion().toString() << std::endl;
```

Subscribe to input and output recipes and set the output frequency to 250Hz. The `out_recipe` and `in_recipe` data type is `std::shared_ptr<RtsiRecipe>`.
```cpp
    auto out_recipe = rtsi->setupOutputRecipe({"timestamp", "actual_joint_positions"}, 250);
    auto in_recipe = rtsi->setupInputRecipe({"speed_slider_mask", "speed_slider_fraction"});
```

Call the `start()` method to send a start synchronization signal. If this is not sent, data cannot be received or set.
```cpp
    if(rtsi->start()) {
        std::cout << "RTSI sync start successful" << std::endl;
    } else {
        std::cout << "RTSI sync start fail" << std::endl;
        return 1;
    }
```

Receive data 250 times and print it.
```cpp
    while(count--) {
        rtsi->receiveData(out_recipe_list);
        out_recipe->getValue("timestamp", timestamp);
        out_recipe->getValue("actual_joint_positions", actula_joints);
        std::cout << "timestamp: " << timestamp << std::endl;
        std::cout << "actual_joint_positions: ";
        for(auto i : actula_joints) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
```

Set the input subscription data and send it to the robot.
```cpp
    in_recipe->setValue("speed_slider_mask", 1);
    in_recipe->setValue("speed_slider_fraction", 0.5);
    rtsi->send(in_recipe);
```

## RtsiRecipe Usage
From the above [Usage of RtsiClientInterface](#RtsiClientInterface-Usage), we can see the usage of `RtsiRecipe`. It should be noted that instances of this class can only be obtained through `setupOutputRecipe()` and `setupInputRecipe()`.

## RtsiIOInterface Usage

Create a code file:
```bash
touch rtsi_io_example.cpp
```

Copy the following code and paste it into `rtsi_io_example.cpp` using your preferred text editor:
```cpp
#include <memory>
#include <iostream>
#include <Elite/RtsiIOInterface.hpp>

using namespace ELITE;

const std::string DEFAULT_ROBOT_IP = "192.168.51.244";

using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }

    std::unique_ptr<RtsiIOInterface> io_interface = std::make_unique<RtsiIOInterface>("output_recipe.txt", "input_recipe.txt", 250);

    if(!io_interface->connect(robot_ip)) {
        std::cout << "Couldn't connect RTSI server" << std::endl;
        return 1;
    }

    std::cout << "Controller is: " << io_interface->getControllerVersion().toString() << std::endl;

    if(!io_interface->setStandardDigital(0, 1)) {
        std::cout << "Set standard digital success" << std::endl;
    }
    std::this_thread::sleep_for(50ms);
    
    uint32_t digital = io_interface->getDigitalOutputBits();

    std::cout << "Digital output :" << digital << std::endl;

    io_interface->disconnect();

    return 0;
}
```

Compile `rtsi_io_example.cpp` using the gcc compiler:
```bash
g++ rtsi_io_example.cpp -o rtsi_io_example -lelite-cs-series-sdk
```

Before executing the compilation result, create two recipe files:
```bash
touch input_recipe.txt output_recipe.txt
```

Copy the following input recipe into `input_recipe.txt`:
```
standard_digital_output_mask
standard_digital_output
```

Copy the following input recipe into `output_recipe.txt`:
```
actual_digital_output_bits
```

Run the compiled binary program:
```bash
./rtsi_io_example <your robot IP>
```

If normal, the following print will be seen:
```
Controller is: 2.13.0.11413
Digital output :1
```
> Note: The version information and the value of the digital output will change according to the actual situation.

## Examine the code
At the beginning of the code, there are header files for the C++ standard. After the standard header files is `Elite/RtsiIOInterface.hpp`, and after including this header file, the basic interface of RTSI can be used.
```cpp
#include <memory>
#include <iostream>
#include <Elite/RtsiIOInterface.hpp>

using namespace ELITE;
```

A smart pointer is created and an instance of `RtsiIOInterface` is created in memory. When creating an instance, the files for the input and output recipes need to be provided, namely "output_recipe.txt" and "input_recipe.txt".
```cpp
std::unique_ptr<RtsiIOInterface> io_interface = std::make_unique<RtsiIOInterface>("output_recipe.txt", "input_recipe.txt", 250);
```

Call the `connect()` method to connect to the robot. The `connect()` method of this class, in addition to TCP connection to the robot, also performs operations such as recipe subscription and starting the synchronization thread.
```cpp
    if(!io_interface->connect(robot_ip)) {
        std::cout << "Couldn't connect RTSI server" << std::endl;
        return 1;
    }
```

Obtain the controller version and print it. Set the digital output 0 of the robot to true, obtain all digital output values and print them.
```cpp
    std::cout << "Controller is: " << io_interface->getControllerVersion().toString() << std::endl;

    if(!io_interface->setStandardDigital(0, 1)) {
        std::cout << "Set standard digital success" << std::endl;
    }
    std::this_thread::sleep_for(50ms);
    
    uint32_t digital = io_interface->getDigitalOutputBits();

    std::cout << "Digital output :" << digital << std::endl;
```

Finally, call `disconnect()` to disconnect from the robot and end the synchronization thread.
```bash
    io_interface->disconnect();
```