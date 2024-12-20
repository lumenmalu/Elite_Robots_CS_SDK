# Dashboard

## Calss
- DashboardClient

## Background
Dashbaord Shell是与机器人交互的一种方式，通过TCP连接机器人的29999端口，可实现机器人上下电、释放抱闸、加载查询任务等操作。SDK中提供了`DashboardClient`类，里面包含了大部分的dashboard接口。

## Prerequisites
- 确保正确安装了SDK。

## Usage

创建一个代码文件：
```bash
touch dashboard_example.cpp
```

复制下面代码，使用你习惯的文本编辑器，粘贴到`dashboard_example.cpp`中：

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

使用gcc编译器，编译`dashboard_example.cpp`：
```bash
g++ dashboard_example.cpp -o dashboard_example -lelite-cs-series-sdk
```

运行编译出来的二进制程序：
```bash
./dashboard_example <your robot IP>
```

如果一切正常应当输出：
```
Connect to robot
Echo right response
```

## Examine the code

代码最开始的部分，包含了C++标准的头文件。标准头文件之后是`Elite/DashboardClient.hpp`，包含了此头文件之后便可以使用dashboard的接口。
```cpp
#include <iostream>

#include <Elite/DashboardClient.hpp>

using namespace ELITE;
```

main函数是程序开始的地方，main函数中最开始的部分的功能是：如果运行`dashboard_example`时，后面有跟机器人的IP参数，则使用此IP，否则使用默认IP。

```cpp
int main(int argc, char* argv[]) {
    std::string robot_ip = DEFAULT_ROBOT_IP;
    if (argc > 1) {
        robot_ip = std::string(argv[1]);
    }
```

紧接着创建了一个智能指针，并在内存中创建了一个`DashboardClient`的实例：
```cpp
    std::unique_ptr<DashboardClient> my_dashboard;
    my_dashboard.reset(new DashboardClient());
```

通过`->`符号可以调用`DashboardClient`的方法，调用`connect()`方法连接到机器人，调用`echo()`方法运行dashboard shell的`echo`指令，如果机器人正确相应了，此方法会返回`true`，依据返回值进行输出，最后调用`disconnect()`方法来断开与dashboard的连接。

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
