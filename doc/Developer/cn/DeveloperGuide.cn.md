<style>
p {
  text-indent: 2em;
}
</style>
# 模块
本库可分为以下几个模块：
- `RTSI`：提供了RTSI客户端的接口，以及进一步获取（或设置）机器人状态的封装接口。
- `Dashboard`：提供了机器人dashboard的接口。
- `Primary Port` ： 提供部分数据的解析，以及脚本发送的接口。
- `EliDriver`：提供控制机器人的接口，需要和`ExternalControl`插件搭配一起使用。


## RTSI 
此模块提供了三个类`RtsiClientInterface`、`RtsiRecipe`和`RtsiIOInterface`，分别位于头文件`RtsiClientInterface.hpp`、`RtsiRecipe.hpp`和`RtsiIOInterface.hpp`之中。

`RtsiClientInterface`类中提供了RTSI客户端的基础接口，例如：`connect()` 、`getControllerVersion()`等。值得提及的是，此类的配置输入、输出订阅配方时，如果成功会返回一个`RtsiRecipe`类的指针，`RtsiRecipe`类只能通过该方式获取，不能手动创建。在使用`receiveData()`接受数据时传入的参数是一个由`RtsiRecipe`类构成的数组，`receiveData()`会接收一个配方并依据ID判断此配方是否在传入的数组中，如果存在则更新数组中对应配方的数据，并返回ID值。可参考以下使用示例：
```cpp
// ... 省略部分代码
int main() {
    RtsiClientInterface* rtsi = new RtsiClientInterface();
    // 连接到机器人
    rtsi->connect(robot_ip);
    // RTSI 版本校验
    rtsi->negotiateProtocolVersion();
    // 获取控制器版本并打印
    std::cout << rtsi->getControllerVersion().to_string() << std::endl;
    // 配置输出配方
    auto outreciep1 = rtsi->setupOutputRecipe({"payload_mass", "payload_cog"});
    auto outreciep2 = rtsi->setupOutputRecipe({"script_control_line", "timestamp"});
    // 配置输入配方
    auto inrecipe = rtsi->setupInputRecipe({"speed_slider_mask", "speed_slider_fraction"});
    // 将两个输出配方构建为一个数组以便后面接收
    auto outrecipe_list = std::vector<std::shared_ptr<RtsiRecipe>>(){outreciep1, outreciep2};
    // 发送开始同步信号
    rtsi->start();
    // 循环100次接受输出配方的数据，并依据ID打印收到哪个配方
    int count = 100;
    while(count--) {
        int id = rtsi->receiveData(outrecipe_list);
        if(id == outreciep1->getID()) {
            std::cout << "收到配方1" << std::endl;
        } else if(id == outreciep2->getID()) {
            std::cout << "收到配方2" << std::endl;
        }
    }
    // 设置输入配方的值并发送
    inrecipe->setValue("speed_slider_mask", 1);
    inrecipe->setValue("speed_slider_fraction", 1.0);
    rtsi->send(inrecipe);
    // 发送同步暂停信号
    rtsi->pause();
    return 0;
}
```

`RtsiIOInterface`继承自`RtsiClientInterface`，在此基础上封装了一些设置、获取机器人状态的接口。`RtsiIOInterface`类在构造时需要提供输入、输出订阅配方的两个文本文件，文件中每个订阅项独占一行，可以参考`example/resource`目录下的文件。  

`RtsiIOInterface`类在调用`connect()`接口成功后会创建一个后台线程，来接收和发送机器人的状态数据。用户可以直接调用现有的接口来获取、设置机器人的状态，可参考`example/rtsi_example.cpp`的示例。值得注意的是，如果配方文件里没有某个订阅项（例如：浮点寄存器0），但又调用了相应的接口（例如：getOutDoubleRegister(0)），此时接口将不会返回正确的数值。

`RtsiClientInterface`相比`RtsiIOInterface`更灵活，使用自由度更高，而`RtsiIOInterface`更加简洁和易用。

## Dashboard
此模块提供了`DashboardClient`类，在头文件`DashboardClient.hpp`中，此类提供了Dashboard的接口，使用可参考`example/dashboard_example.cpp`的示例。

## Primary Port
此模块提供了 `PrimaryPortInterface`类，在头文件PrimaryPortInterface.hpp。此类提供了解析部分30001报文的功能，以及发送自定义脚本的接口。相关的接口也集成到了`EliDriver`类中。  

sdk中没有完全解析30001端口的报文，如果需要其他报文可以写一个自定义的类，继承 `PrimaryPackage` 使用 `PrimaryPortInterface::getPackage()`或`EliDriver::getPrimaryPackage()`接口来获取包的内容。

## EliDriver
### 使用简述
因为`EliDriver`需要和`ExternalControl`插件搭配使用，因此先简述下`ExternalControl`插件的使用：安装插件后在“配置”中找到`ExternalControl`的配置，`Host IP`填入外部控制器（你的PC）的IP，`Custom port`可保持默认的`50002`，`Host name`填入任意名称。配置完成后在任务树中插入`ExternalControl`节点。在外部控制器运行起来后，需要运行任务外部控制器才能控制机器人，当然外部控制器也可以通过`DashboardClient`的接口或者其他方式来启动任务。  

在构造`EliDriver`对象时的参数含义可参考头文件中的注释，但需要注意的是参数中的`xxx_port`代表了会创建一个监听此端口的 TCP server ，因此需要确保系统中没有其他程序监听此端口，如果有，可以考虑改变端口，如果改变了`script_sender_port`则需要在`ExternalControl`插件的配置中修改`Custom port`为一致的。  

当前版本(V2.12)的`EliDriver`提供了两类运动接口，第一类接口如下：
- `writeServoj()`：对应`servoj()`指令。
- `writeSpeedl()`：对应`speedl()`指令。
- `writeSpeedj()`：对应`speedj()`指令。
- `writeIdle()`：令机器人停止运动或保持静止。

以上接口的含义可参考注释，需要注意的是`timeout_ms`参数，如果下一次调用没有在`timeout_ms`的时间之内，会导致机器人退出外部控制，这么做的目的是为了保证实时性。以`writeServoj()`为例，可简单参考下面的例子：
```cpp
// ...省略部分代码
// target_positions 是省略代码中构建的一个关节轨迹，这里将每个点位通过 writeServoj() 发送
for (auto& target : target_positions) {
    // 设置 100ms 超时，所以需要在 100ms 内发送下一个点位
    if(!s_driver->writeServoj(target, 100)) {
        return 1;
    }
    std::this_thread::sleep_for(4ms);
}
// ...省略部分代码
```

详细的示例用法可以参考`example/`目录下的`servo_example.cpp`、`speedj_example.cpp`、`speedl_example.cpp`。  

第二类接口，也可以看作一整套，对应了`movej()`和`movel()`指令
- `writeTrajectoryPoint()`：发送目标点位、转接半径、运行时间、运动方式的数据，运动方式指使用`movej()`还是`movel()`。
- `writeTrajectoryControlAction()`：发送开始、取消、NOOP指令。需要注意，此接口和第一类接口一样有`timeout_ms`参数，因此需要在`timeout_ms`时间之前给出下一步操作，否则也会导致机器人退出外部控制。在等待机器人到达点位的过程中可以发送NOOP指令。
- `setTrajectoryResultCallback()`：接收机器人的运动结果。

可以简单参考下面的示例：
```cpp
// ...省略部分代码
// 当运动完成、失败、被取消时，会调用lambda表达式：如果运动成功设置is_move_finish标志为true
bool is_move_finish = false;
s_driver->setTrajectoryResultCallback([&](TrajectoryMotionResult result){
    if (result == TrajectoryMotionResult::SUCCESS) {
        is_move_finish = true;
    }
});
// 发送开始指令并设置200ms超时，机器人将开始接收外部控制器的1个点位并运动（一个点位对应第二个参数“1”）
s_driver->writeTrajectoryControlAction(ELITE::TrajectoryControlAction::START, 1, 200);
// 发送1个目标点位，3秒到达，转接半径为0，使用关节空间（对应movej，笛卡尔空间对应movel）
s_driver->writeTrajectoryPoint(target_joints, 3, 0, false);
// 等待运动结束，等待期间持续发送NOOP指令来“保活”
while (!is_move_finish) {
    s_driver->writeTrajectoryControlAction(ELITE::TrajectoryControlAction::NOOP, 0, 200);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
// ...省略部分代码
```

详细的示例用法可以参考`example/`目录下的`trajectory_example.cpp`。  

### 工作原理简述
此部分内容讲述了`EliDriver`是如何控制机器人的。在说明`EliDriver`原理前需要先讲述`ExternalControl`插件的工作流程。

#### `ExternalControl`插件工作流
简单来说`ExternalControl`是：“请求脚本--->接收与处理脚本--->运行脚本”的流程，在开始运行任务后，插件会向外部控制器发送`request\n`字符串来请求脚本，外部控制器在接收到请求后会向插件发送脚本，脚本的格式如下：
```python
# HEADER_BEGIN
...
# HEADER_END

# NODE_CONTROL_LOOP_BEGINS
...
# NODE_CONTROL_LOOP_ENDS

```
插件依据以上格式将脚本分割为“header”和“control loop”两个部分，这两个部分的内容分别是“声明”和“执行”，例如：
```python
# HEADER_BEGIN
# 声明了printHello()函数
def printHello():
    textmsg("Hello")
# HEADER_END

# NODE_CONTROL_LOOP_BEGINS
# 执行printHello()
printHello()
# NODE_CONTROL_LOOP_ENDS

```
可以猜测到，外部控制器需要创建一个TCP的Server，端口就对应着插件配置的`Custom port`，并且在接收到`request\n`字符串时需要回复一个符合格式要求的脚本。下图简述了整个工作流：
<div style="text-align: center;">
    <img src="ExternalControl.drawio.png" alt="Description"/>
</div>

#### `EliDriver`架构
既然`ExternalControl`插件是接收脚本、运行脚本的流程，因此运行实际的控制指令也是在该脚本中的，所以我们直接来看脚本中`NODE_CONTROL_LOOP_BEGINS`后面的内容，脚本最开始调用了`socket_open()`创建了几个socket，主要看`reverse_socket`（IP和port部分可以先暂时不管，后文会说明），这个socket主要是接收外部控制器的运动指令和数据，接着往后看能看到一个`while`循环，这里面就是机器人的控制循环，通过`reverse_socket`接收到的报文来获取指令和点位或者速度等信息。以下是此循环的流程：
<div style="text-align: center;">
    <img src="ControlScript.drawio.png" alt="Description"/>
</div>

通过上面的流程图我们看到`Run command`部分是依据`reverse_socket`接收到的数据来执行运动指令的。我们现在来看`EliDriver`中的`writeServoj()`、`writeSpeedl()`、`writeSpeedj()`、`writeIdle()`接口，对于这几个接口来说，发送的内容都是“指令+点位或速度”，脚本在接收到指令后，会启动一个线程来执行运动，而主线程会保持接收`reverse_socket`的数据来更新运动数据或者模式。  

在脚本中注意到另外的一个socket——`trajectory_socket`。这个socket对应了`writeTrajectoryPoint()`、`writeTrajectoryControlAction()`、`setTrajectoryResultCallback()` 这一套接口，其和`reverse_socket`的工作流程图如下：
<div style="text-align: center;">
    <img src="ControlScript-trajectory_socket.drawio.png" alt="Description"/>
</div>

可以看到`Trajectory`接口的工作流程是`reverse_socket`在负责接收指令，在接收到`Trajectory`指令的 start action 后会启动一个线程，此线程的`trajectory_socket`负责接收点位并且运动到点位。  

## 总结
下图展示了整个SDK与机器人的数据流向图：
<div style="text-align: center;">
    <img src="SDKDataFlow.drawio.png" alt="Description"/>
</div>
