# EliteDriver 类

## 简介

EliteDriver 是用于与机器人进行数据交互的主要类。它负责建立所有必要的套接字连接，并处理与机器人的数据交换。EliteDriver 会向机器人发送控制脚本，机器人在运行控制脚本后，会和 EliteDriver 建立通讯，接收运动数据，并且必要时会发送运动结果。

## 头文件
```cpp
#include <Elite/EliteDriver.hpp>
```

## 构造与析构函数

- ***构造函数***
    ```cpp
    EliteDriver::EliteDriver(
        const std::string& robot_ip, 
        const std::string& local_ip, 
        const std::string& script_file,
        bool headless_mode = false, 
        int script_sender_port = 50002, 
        int reverse_port = 50001,
        int trajectory_port = 50003, 
        int script_command_port = 50004, 
        float servoj_time = 0.008,
        float servoj_lookhead_time = 0.08, 
        int servoj_gain = 300, 
        float stopj_acc = 4.0);
    ```
    - ***功能***

        创建 EliteDriver 对象，并初始化与机器人通信的必要连接。

    - ***参数***
        - robot_ip：机器人 IP 地址。

        - local_ip：本机 IP 地址。

        - script_file：控制脚本模板文件。

        - headless_mode：是否以无界面模式运行。如果此参数为true，那么在构造函数中，将会向机器人的 primary 端口发送一次控制脚本。

        - script_sender_port：发送脚本的端口。

        - reverse_port：反向通信端口。

        - trajectory_port：发送轨迹点的端口。

        - script_command_port：发送脚本命令的端口。

        - servoj_time：伺服运动的时间参数。

        - servoj_lookhead_time：伺服运动前瞻时间，范围 [0.03, 0.2] 秒。

        - servoj_gain：伺服增益。

        - stopj_acc：停止运动的加速度 (rad/s²)。

- ***析构函数***
    ```cpp
    ~EliteDriver::EliteDriver()
    ```
    - ***功能***

        释放资源

## 运动控制

- ***writeServoj***
    ```cpp
    bool writeServoj(const vector6d_t& pos, int timeout_ms)
    ```
    - ***功能***

        向机器人发送伺服运动的指令。

    - ***参数***
        - pos：目标点位

        - timeout_ms：设置机器人读取下一条指令的超时时间，小于等于0时会无限等待。
    
    - ***返回值***：成功返回 true，失败返回 false。

- ***writeSpeedl***
    ```cpp
    bool writeSpeedl(const vector6d_t& vel, int timeout_ms)
    ```
    - ***功能***

         向机器人发送线速度控制指令。

    - ***参数***
        - vel：线速度 [x, y, z, rx, ry, rz]。

        - timeout_ms：设置机器人读取下一条指令的超时时间，小于等于0时会无限等待。
    
    - ***返回值***：成功返回 true，失败返回 false。

- ***writeIdle***
    ```cpp
    bool writeIdle(int timeout_ms)
    ```
    - ***功能***

        发送空闲指令，如果机器人正在运动会使机器人停止运动。

    - ***参数***
        - timeout_ms：设置机器人读取下一条指令的超时时间，小于等于0时会无限等待。

    - ***返回值***：成功返回 true，失败返回 false。

## 轨迹运动
- ***setTrajectoryResultCallback***
    ```cpp
    void setTrajectoryResultCallback(std::function<void(TrajectoryMotionResult)> cb)
    ```
    - ***功能***

        注册轨迹完成时的回调函数。
        控制机器人的一种方式是将路点一次性发给机器人，当执行完成时，这里注册的回调函数将被触发。

    - ***参数***
        - cb：执行完成时的回调函数

- ***writeTrajectoryPoint***
    ```cpp
    bool writeTrajectoryPoint(const vector6d_t& positions, float time, float blend_radius, bool cartesian)
    ```
    - ***功能***

        向专门的socket写入轨迹路点。

    - ***参数***
        - positions：路点
        
        - time：到达路点的时间
        
        - blend_radius：两个路点的转接半径

        - cartesian：如果发送的点是笛卡尔的，则为True，如果是基于关节的，则为false

    - ***返回值***：成功返回 true，失败返回 false。

- ***writeTrajectoryControlAction***
    ```cpp
    bool writeTrajectoryControlAction(TrajectoryControlAction action, const int point_number, int timeout_ms)
    ```
    - ***功能***

        发送轨迹控制指令。

    - ***参数***
        - action：轨迹控制的动作。

        - point_number：路点的数量。

        - timeout_ms：设置机器人读取下一条指令的超时时间，小于等于0时会无限等待。

    - ***返回值***：成功返回 true，失败返回 false。

    - ***注意***：写入`START`动作之后，需要在超时时间内写入下一条指令，可以写入`NOOP`。

## 机器人配置
- zeroFTSensor
    ```cpp
    bool zeroFTSensor()
    ```
    - ***功能***

        将力/力矩传感器测量的施加在工具 TCP 上的力/力矩值清零（去皮），所述力/力矩值为 get_tcp_force(True) 脚本指令获取的施加在工具 TCP 上的力/力矩矢量，该矢量已进行负载补偿等处理。
        该指令执行后，当前的力/力矩测量值会被作为力/力矩参考值保存，后续所有的力/力矩测量值都会减去该力/力矩参考值（去皮）。
        请注意，上述力/力矩参考值会在该指令执行时更新，在控制器重启后将重置为 0。

    - ***返回值***：成功返回 true，失败返回 false。

- setPayload
    ```cpp
    bool setPayload(double mass, const vector3d_t& cog)
    ```
    - ***功能***

        该命令用于设置机器人载荷的质量、重心和转动惯量。

    - ***参数***
        - mass：负载质量

        - cog：有效载荷的重心坐标（相对于法兰框架）。

    - ***返回值***：成功返回 true，失败返回 false。

- setToolVoltage
    ```cpp
    bool setToolVoltage(const ToolVoltage& vol)
    ```
    - ***功能***
        设置工具电压
    
    - ***参数***
        - vol：工具电压

    - ***返回值***：成功返回 true，失败返回 false。



## 其余
- stopControl
    ```cpp
    bool stopControl()
    ```
    - ***功能***

        发送停止指令到机器人，机器人将退出控制脚本，并且将停止接收来自PC的指令。

    - ***返回值***：成功返回 true，失败返回 false。

- isRobotConnected
    ```cpp
    bool isRobotConnected()
    ```
    - ***功能***

        是否和机器人连接上

    - ***返回值***：成功返回 true，失败返回 false。

- 