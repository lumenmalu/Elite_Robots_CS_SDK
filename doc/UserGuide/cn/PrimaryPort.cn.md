# Primary Port

## Class
- PrimaryPortInterface
- PrimaryPackage

## Background
Primary port是 Elite CS 系列机器人的一种通讯接口，以10HZ的频率向外发送机器人的状态数据，同时此接口能接收脚本并运行。协议的具体说明可到官网下载说明文档。

## Prerequisites
- 确保正确安装了SDK。

## Usage

创建一个代码文件：
```bash
touch primary_example.cpp
```

复制下面代码，使用你习惯的文本编辑器，粘贴到`primary_example.cpp`中：
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
    // In version 2.11.0 of robot controller, the DH parameters are located at a specific offset relative to the sub-header of the message. 
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

使用gcc编译器，编译`primary_example.cpp`：
```bash
g++ primary_example.cpp -o primary_example -lelite-cs-series-sdk
```

执行编译结果
```
./primary_example <robot ip>
```

正常的话应该会看到机器人的DH参数被打印：
```
DH parameter a: 0       0       -0.427  -0.3905 0       0
DH parameter d: 0.1625  0       0       0.1475  0.0965  0.092
DH parameter alpha: 0   1.5708  0       0       1.5708  -1.5708
```
查看机器人示教器的界面，在日志部分应当会出现`hello world`的消息。

## Examine the code

代码的开头部分是C++的头文件，`PrimaryPortInterface.hpp`让我们可以使用 Primary Port 接口功能，包括了package报文获取、脚本发送。`PrimaryPackage.hpp`包含了Primary Port包的抽象类。因为并不需要Primary Port的全部数据，所以SDK中并未提供所有报文的解析。`DataType.hpp`头文件里定义了SDK中常用的一些数据结构。剩下的就是C++的标准头文件。
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

接下来的代码演示了如何自己手动编写一个解析子报文的类。首先我们需要从官网下载  Primary port 协议报文的说明表格。
我们想要获取“机器人配置数据子报文”中的DH参数，直接定义`KinematicsInfo`类，这个类必须继承`ELITE::PrimaryPackage`，否则无法推送给 Primary port 解析。`PrimaryPackage`类的构造函数需要输入子报文的类型，查看报文说明表格：“机器人配置数据子报文”的报文类型数字为6，因此定义了一个常量`ROBOT_CONFIG_PKG_TYPE = 6`，在构造函数中，将这个常量传递给`PrimaryPackage`。  
因为`ELITE::PrimaryPackage`是一个抽象类，定义了`parser()`方法必须在其子类中实现，所以，在`KinematicsInfo`中实现了`parser()`函数，此函数的第一个参数是子报文的长度，第二个参数是子报文开始的第一个字节的迭代器。  
因为我们只要DH参数，查看报文说明表格：计算出DH参数第一个字节的位置，与子报文第一个字节的位置的偏移，由此定义了`DH_PARAM_OFFSET`常量。由于Primary port接口的数据是大端序，而Ubuntu系统默认的端序是小端序，因此编写了一个`unpack()`函数来转换。在`parser()`函数中我们依据报文说明表格来逐项解析报文。
```cpp
class KinematicsInfo : public ELITE::PrimaryPackage {
private:
    static constexpr int ROBOT_CONFIG_PKG_TYPE = 6;
    // In version 2.11.0 of robot controller, the DH parameters are located at a specific offset relative to the sub-header of the message. 
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

main函数是程序开始的地方，main函数中最开始的部分的功能是：如果运行`primary_example`时，后面有跟机器人的IP参数，则使用此IP，否则使用默认IP。
```cpp
int main(int argc, const char **argv) {
    // Parse the ip arguments if given
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }
```

创建`PrimaryPortInterface`和`KinematicsInfo`的指针，并实例化他们。
```cpp
auto primary = std::make_unique<ELITE::PrimaryPortInterface>();
    
auto kin = std::make_shared<KinematicsInfo>();
```

primary调用`connect()`方法连接到机器人
```cpp
    primary->connect(robot_ip, 30001);
```

primary调用`getPackage()`方法从机器人获取并解析数据包，此处需要将`kin`变量传入。
```cpp
    primary->getPackage(kin, 200);
```

获取到数据包后打印DH参数。
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

定义一个脚本并发送给Primary port端口，让机器人执行。
```cpp
    std::string script = "sec hello():\n\ttextmsg(\"hello world\")\nend\n";

    primary->sendScript(script);
```

上面示例中`KinematicsInfo`类在SDK中已经写好，引用头文件`RobotConfPackage.hpp`即可。
