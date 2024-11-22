[English](./README.md)
# Elite Robots CS SDK

Elite CS 系列机器人的SDK。

## Requirements
 * **EliRobot & EliServer** (机器人的控制软件) version >=**2.10.x**(for CS-Series). 
   如果机器人的控制软件版本低于此，建议升级之。  
 * SDK中的socket使用了 **boost::asio**。 因此需要安装 **boost** 库。
 * 此SDK需要支持 C++17 或 C++14 的编译器。注意，如果是C++14的标准，会使用到`boost::variant`。
 * cmake版本 >=3.22.1

## Build
在Windows平台上，您可以使用以下步骤编译此项目:
```bash
cd <clone of this repository>
mkdir build && cd build
cmake ..
cmake --build ./
```
编译完成后会得到`libelite_robot_client_static.lib` 和 `libelite_robot_client.dll` 两个库文件，以及包括了头文件的`include`文件夹。

对于Linux平台，可以使用下面步骤编译与安装：
```bash
cd <clone of this repository>
mkdir build && cd build
cmake ..
make -j
sudo make install
```

## User guide
[中文向导](./doc/UserGuide/cn/UserGuide.cn.md)  

## Document
编译文档，需要先安装依赖：
```bash
sudo apt-get update
sudo apt-get install doxygen
sudo apt-get install doxygen-gui
```

使用以下步骤便可以编译文档：
```bash
cd <clone of this repository>
mkdir build && cd build
cmake -DELITE_COMPLIE_DOC=TRUE ..
make -j
```
编译完成后，在`./build`目录下能看到`./docs`目录，里面就包含了使用文档。

## Example
在文件夹`./example/`中，包含了如何使用此SDK的示例。如果需要编译示例，请参考以下步骤：
```bash
cd <clone of this repository>
mkdir build && cd build
cmake -DELITE_COMPLIE_EXAMPLES=TRUE .. 
make -j
sudo make install
```

## Compatible Operating Systems
在以下系统平台上测试过：

 * Ubuntu 22.04 (Jammy Jellyfish)
 * Ubuntu 16.04 (Xenial Xerus)
 * Windowns 11

## Compiler
目前在以下编译器中通过编译

 * gcc 11.4.0
 * gcc 5.5.0
 * msvc 19.40.33808
