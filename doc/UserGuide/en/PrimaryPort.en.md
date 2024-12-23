# Primary Port

## Class
- PrimaryPortInterface
- PrimaryPackage

## Background
The Primary port is a communication interface for Elite CS series robots. It sends robot status data at a frequency of 10Hz and can receive scripts and run them. For the specific description of the protocol, you can download the description document from the official website.

## Prerequisites
- Ensure that the SDK is installed correctly.

## Usage

Create a code file:
```bash
touch primary_example.cpp
```

Copy the following code and paste it into `primary_example.cpp` using your preferred text editor:
```cpp
#include <Elite/PrimaryPortInterface.hpp>
#include <Elite/PrimaryPackage.hpp>
#include <Elite/DataType.hpp>
#include <string>
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono;

const std::string DEFAULT_ROBOT_IP = "192.168.51.244";

class KinematicsInfo : public ELITE::PrimaryPackage {
private:
    static constexpr int ROBOT_CONFIG_PKG_TYPE = 6;
    // In version 2.11.0 of the robot controller, the DH parameters are located at a specific offset relative to the sub-header of the message. 
    static constexpr int DH_PARAM_OFFSET = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(double) * 2 * 6 + sizeof(double) * 2 * 6 + sizeof(double) * 5;

    void unpack(const std::vector<uint8_t>::const_iterator& iter, double& out) {
        union {
            double value;
            uint8_t bytes[sizeof(double)];
        } convert;
        for(int i = 0; i < sizeof(double); i++) {
            convert.bytes[i] = *(iter + (sizeof(double) - i - 1));
        }
        out = convert.value;
    }

public:
    KinematicsInfo() : PrimaryPackage(ROBOT_CONFIG_PKG_TYPE) { };
    ~KinematicsInfo() = default;

    ELITE::vector6d_t dh_a_;
    ELITE::vector6d_t dh_d_;
    ELITE::vector6d_t dh_alpha_;

    void parser(int len, const std::vector<uint8_t>::const_iterator& iter) {
        int offset = DH_PARAM_OFFSET;
        for (size_t i = 0; i < 6; i++) {
            unpack(iter + offset, dh_a_[i]);
            offset += sizeof(double);
        }
        for (size_t i = 0; i < 6; i++) {
            unpack(iter + offset, dh_d_[i]);
            offset += sizeof(double);
        }
        for (size_t i = 0; i < 6; i++) {
            unpack(iter + offset, dh_alpha_[i]);
            offset += sizeof(double);
        }
    }
};

int main(int argc, const char **argv) {
    // Parse the ip arguments if given
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }

    auto primary = std::make_unique<ELITE::PrimaryPortInterface>();
    
    auto kin = std::make_shared<KinematicsInfo>();
    
    primary->connect(robot_ip, 30001);

    primary->getPackage(kin, 200);

    std::cout << "DH parameter a: ";
    for (auto i : kin->dh_a_) {
        std::cout << i << "\t";
    }
    std::cout << std::endl;

    std::cout << "DH parameter d: ";
    for (auto i : kin->dh_d_) {
        std::cout << i << "\t";
    }
    std::cout << std::endl;

    std::cout << "DH parameter alpha: ";
    for (auto i : kin->dh_alpha_) {
        std::cout << i << "\t";
    }
    std::cout << std::endl;

    std::string script = "sec hello():\n\ttextmsg(\"hello world\")\nend\n";

    primary->sendScript(script);

    std::this_thread::sleep_for(1s);

    primary->disconnect();

    return 0;
}
```

Compile `primary_example.cpp` using the gcc compiler:
```bash
g++ primary_example.cpp -o primary_example -lelite-cs-series-sdk
```

Execute the compilation result:
```
./primary_example <robot ip>
```

If everything is normal, the DH parameters of the robot should be printed:
```
DH parameter a: 0       0       -0.427  -0.3905 0       0
DH parameter d: 0.1625  0       0       0.1475  0.0965  0.092
DH parameter alpha: 0   1.5708  0       0       1.5708  -1.5708
```
Check the interface of the robot teach pendant. In the log section, the message "hello world" should appear.

## Examine the code

At the beginning of the code are C++ header files. `PrimaryPortInterface.hpp` allows us to use the Primary Port interface functions, including package message acquisition and script sending. `PrimaryPackage.hpp` contains the abstract class of the Primary Port package. Since not all the data of the Primary Port is needed, the SDK does not provide parsing for all messages. The `DataType.hpp` header file defines some commonly used data structures in the SDK. The rest are C++ standard header files.
```cpp
#include <Elite/PrimaryPortInterface.hpp>
#include <Elite/PrimaryPackage.hpp>
#include <Elite/DataType.hpp>
#include <string>
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono;
```

The following code demonstrates how to manually write a class to parse sub-messages. First, we need to download the description table of the Primary port protocol message from the official website.
We want to obtain the DH parameters in the "Robot Configuration Data Sub-message". We directly define the `KinematicsInfo` class. This class must inherit from `ELITE::PrimaryPackage`, otherwise it cannot be pushed for Primary port parsing. The constructor of the `PrimaryPackage` class requires the type of the sub-message. Check the message description table: The message type number of the "Robot Configuration Data Sub-message" is 6. Therefore, a constant `ROBOT_CONFIG_PKG_TYPE = 6` is defined. In the constructor, this constant is passed to `PrimaryPackage`.
Since `ELITE::PrimaryPackage` is an abstract class and the `parser()` method must be implemented in its subclass, the `parser()` function is implemented in `KinematicsInfo`. The first parameter of this function is the length of the sub-message, and the second parameter is the iterator of the first byte at the beginning of the sub-message.
Since we only want the DH parameters, check the message description table: Calculate the offset between the position of the first byte of the DH parameters and the position of the first byte of the sub-message. Thus, the constant `DH_PARAM_OFFSET` is defined. Since the data of the Primary port interface is in big-endian order, while the default endianness of the Ubuntu system is little-endian, an `unpack()` function is written to convert. In the `parser()` function, we parse the message item by item according to the message description table.
```cpp
class KinematicsInfo : public ELITE::PrimaryPackage {
private:
    static constexpr int ROBOT_CONFIG_PKG_TYPE = 6;
    // In version 2.11.0 of the robot controller, the DH parameters are located at a specific offset relative to the sub-header of the message. 
    static constexpr int DH_PARAM_OFFSET = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(double) * 2 * 6 + sizeof(double) * 2 * 6 + sizeof(double) * 5;

    void unpack(const std::vector<uint8_t>::const_iterator& iter, double& out) {
        union {
            double value;
            uint8_t bytes[sizeof(double)];
        } convert;
        for(int i = 0; i < sizeof(double); i++) {
            convert.bytes[i] = *(iter + (sizeof(double) - i - 1));
        }
        out = convert.value;
    }

public:
    KinematicsInfo() : PrimaryPackage(ROBOT_CONFIG_PKG_TYPE) { };
    ~KinematicsInfo() = default;

    ELITE::vector6d_t dh_a_;
    ELITE::vector6d_t dh_d_;
    ELITE::vector6d_t dh_alpha_;

    void parser(int len, const std::vector<uint8_t>::const_iterator& iter) {
        int offset = DH_PARAM_OFFSET;
        for (size_t i = 0; i < 6; i++) {
            unpack(iter + offset, dh_a_[i]);
            offset += sizeof(double);
        }
        for (size_t i = 0; i < 6; i++) {
            unpack(iter + offset, dh_d_[i]);
            offset += sizeof(double);
        }
        for (size_t i = 0; i < 6; i++) {
            unpack(iter + offset, dh_alpha_[i]);
            offset += sizeof(double);
        }
    }
};
```

The main function is where the program starts. The function of the beginning part of the main function is: if there is a robot IP parameter after running `primary_example`, use this IP; otherwise, use the default IP.
```cpp
int main(int argc, const char **argv) {
    // Parse the ip arguments if given
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }
```

Create pointers to `PrimaryPortInterface` and `KinematicsInfo` and instantiate them.
```cpp
auto primary = std::make_unique<ELITE::PrimaryPortInterface>();
    
auto kin = std::make_shared<KinematicsInfo>();
```

The `primary` calls the `connect()` method to connect to the robot.
```cpp
    primary->connect(robot_ip, 30001);
```

The `primary` calls the `getPackage()` method to obtain and parse the data packet from the robot. Here, the `kin` variable needs to be passed in.
```cpp
    primary->getPackage(kin, 200);
```

After obtaining the data packet, print the DH parameters.
```cpp
    std::cout << "DH parameter a: ";
    for (auto i : kin->dh_a_) {
        std::cout << i << "\t";
    }
    std::cout << std::endl;

    std::cout << "DH parameter d: ";
    for (auto i : kin->dh_d_) {
        std::cout << i << "\t";
    }
    std::cout << std::endl;

    std::cout << "DH parameter alpha: ";
    for (auto i : kin->dh_alpha_) {
        std::cout << i << "\t";
    }
    std::cout << std::endl;
```

Define a script and send it to the Primary port so that the robot can execute it.
```cpp
    std::string script = "sec hello():\n\ttextmsg(\"hello world\")\nend\n";

    primary->sendScript(script);
```

In the example above, the `KinematicsInfo` class has already been written in the SDK. Just include the header file `RobotConfPackage.hpp`.
