# RTSI

## Class
- RtsiClientInterface : 提供了RTSI基础的操作接口。
- RtsiRecipe : 提供了RTSI配方的操作接口。
- RtsiIOInterface：对RTSI订阅项进行了封装。

## Background
RTSI 是 Elite CS 系列机器人的一种通讯接口，最高可以250HZ的频率读取、写入机器人IO。SDK中提供了两种接口，第一种接口是RTSI的基础接口（RtsiClientInterface），可以操控RTSI通讯的每一个步骤。第二种接口将机器人的各种数据（RtsiIOInterface），例如关节角等，封装成了函数，可以直接调用。  
RTSI 协议的具体说明可到官网下载RTSI说明文档。

## Prerequisites
- 确保正确安装了SDK。

## RtsiClientInterface使用

创建一个代码文件：
```bash
touch rtsi_base_example.cpp
```

复制下面代码，使用你习惯的文本编辑器，粘贴到`rtsi_base_example.cpp`中：

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

使用gcc编译器，编译`rtsi_base_example.cpp`：
```bash
g++ rtsi_base_example.cpp -o rtsi_base_example -lelite-cs-series-sdk
```

运行编译出来的二进制程序：
```bash
./rtsi_base_example <your robot IP>
```

如果一切正常应当输出时间戳和实际的机器人关节角度。
> 注意：如果程序抛出异常`rtsi unknow variable type: variable "speed_slider_mask" error type: IN_USE`建议检查是否有其他的RTSI连接或总线连接。

## Examine the code
代码最开始的部分，包含了C++标准的头文件。标准头文件之后是`Elite/RtsiClientInterface.hpp`，包含了此头文件之后便可以使用RTSI的基础接口。在这之后是`Elite/RtsiRecipe.hpp`，此头文件提供了RTSI的配方操作方法。
```cpp
#include <iostream>
#include <Elite/RtsiClientInterface.hpp>
#include <Elite/RtsiRecipe.hpp>

using namespace ELITE;
```

main函数是程序开始的地方，main函数中最开始的部分的功能是：如果运行`rtsi_base_example`时，后面有跟机器人的IP参数，则使用此IP，否则使用默认IP。
```cpp
int main(int argc, char* argv[]) {
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }
```

紧接着创建了一个智能指针，并在内存中创建了一个`RtsiClientInterface`的实例：
```cpp
    std::unique_ptr<RtsiClientInterface> rtsi = std::make_unique<RtsiClientInterface>();
```

通过`->`符号可以调用`RtsiClientInterface`的方法，调用`connect()`方法连接到机器人：
```cpp
    rtsi->connect(robot_ip);
```

下面的代码是验证RTSI的协议版本，在通讯开始之前必须进行这一步，否则无法进行后面的通讯：
```cpp
    if(rtsi->negotiateProtocolVersion()) {
        std::cout << "Negotiate protocol version success" << std::endl;
    } else {
        std::cout << "Negotiate protocol version fail" << std::endl;
        return 1;
    }
```

使用`getControllerVersion()`方法获取控制器版本，并打印
```cpp
    std::cout << "Controller version: " << rtsi->getControllerVersion().toString() << std::endl;
```

订阅输入、输出配方，并设置输出频率为250HZ，这里的`out_recipe`、`in_recipe`的类型是`std::shared_ptr<RtsiRecipe>`。
```cpp
    auto out_recipe = rtsi->setupOutputRecipe({"timestamp", "actual_joint_positions"}, 250);
    auto in_recipe = rtsi->setupInputRecipe({"speed_slider_mask", "speed_slider_fraction"});
```

调用`start()`方法发送开始同步的信号，如果没有发送则无法接收、设置数据。
```cpp
    if(rtsi->start()) {
        std::cout << "RTSI sync start successful" << std::endl;
    } else {
        std::cout << "RTSI sync start fail" << std::endl;
        return 1;
    }
```

接收250次数据并打印。
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

设置输入订阅数据，并发送给机器人。
```cpp
    in_recipe->setValue("speed_slider_mask", 1);
    in_recipe->setValue("speed_slider_fraction", 0.5);
    rtsi->send(in_recipe);
```

## RtsiRecipe 使用
通过上文"RtsiClientInterface使用"中可以看到`RtsiRecipe`的使用，需要注意的是，这个类的实例只能通过`setupOutputRecipe()`和`setupInputRecipe()`获得。

## RtsiIOInterface 使用

创建一个代码文件：
```bash
touch rtsi_io_example.cpp
```

复制下面代码，使用你习惯的文本编辑器，粘贴到`rtsi_io_example.cpp`中：
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

使用gcc编译器，编译`rtsi_io_example.cpp`：
```bash
g++ rtsi_io_example.cpp -o rtsi_io_example -lelite-cs-series-sdk
```

在执行编译结果之前，创建两个配方文件：
```bash
touch input_recipe.txt output_recipe.txt
```

将下面的输入配方复制到`input_recipe.txt`中：
```
standard_digital_output_mask
standard_digital_output
```

将下面的输入配方复制到`output_recipe.txt`中：
```
actual_digital_output_bits
```

运行编译出来的二进制程序：
```bash
./rtsi_io_example <your robot IP>
```

如果正常，将会看到下面的打印：
```
Controller is: 2.13.0.11413
Digital output :1
```
> 注意：版本信息和数字输出的值根据实际情况会有变化。

## Examine the code
代码最开始的部分，包含了C++标准的头文件。标准头文件之后是`Elite/RtsiIOInterface.hpp`，包含了此头文件之后便可以使用RTSI的基础接口。
```cpp
#include <memory>
#include <iostream>
#include <Elite/RtsiIOInterface.hpp>

using namespace ELITE;
```

main函数是程序开始的地方，main函数中最开始的部分的功能是：如果运行`rtsi_io_example`时，后面有跟机器人的IP参数，则使用此IP，否则使用默认IP。

```cpp
int main(int argc, char* argv[]) {
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }
```

创建了一个智能指针，并在内存中创建了一个`RtsiIOInterface`的实例，创建实例时需要提供输入、输出配方的文件，即"output_recipe.txt", "input_recipe.txt"。
```cpp
std::unique_ptr<RtsiIOInterface> io_interface = std::make_unique<RtsiIOInterface>("output_recipe.txt", "input_recipe.txt", 250);
```

调用`connect()`方法连接机器人，此类的`connect()`方法，除了TCP连接到机器人外，还进行了配方订阅、启动同步线程等操作。
```cpp
    if(!io_interface->connect(robot_ip)) {
        std::cout << "Couldn't connect RTSI server" << std::endl;
        return 1;
    }
```

获取控制器版本，并打印。设置机器人的数字输出0为true，获取所有的数字输出值并打印
```cpp
    std::cout << "Controller is: " << io_interface->getControllerVersion().toString() << std::endl;

    if(!io_interface->setStandardDigital(0, 1)) {
        std::cout << "Set standard digital success" << std::endl;
    }
    std::this_thread::sleep_for(50ms);
    
    uint32_t digital = io_interface->getDigitalOutputBits();

    std::cout << "Digital output :" << digital << std::endl;
```

最后调用`disconnect()`断开与机器人的链接，并且结束同步线程。
```bash
    io_interface->disconnect();
```
