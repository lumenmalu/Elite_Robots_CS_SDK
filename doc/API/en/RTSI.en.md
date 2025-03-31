# RTSI

## Introduction
RTSI is the real-time communication interface of Elite robots, which can be used to obtain robot status, set I/O, etc. The SDK provides two interfaces for RTSI: `RtsiClientInterface` and `RtsiIOInterface`. `RtsiClientInterface` requires manual operations such as connection and version verification. `RtsiIOInterface` encapsulates most of the interfaces. In actual testing, the real-time performance of `RtsiIOInterface` is slightly worse, while the real-time performance of `RtsiClientInterface` depends on the user's code.

# RtsiClientInterface Class

## Introduction
RTSI client

## Header File
```cpp
#include <Elite/RtsiClientInterface.hpp>
```

## Interfaces

### ***Connection***
```cpp
void connect(const std::string& ip, int port = 30004)
```
- ***Function***
Connects to the robot's RTSI port.
- ***Parameters***
    - ip: The IP address of the robot.
    - port: The RTSI port.

---

### ***Disconnection***
```cpp
void disconnect();
```
- ***Function***
Disconnects from the robot.

---

### ***Protocol Version Verification***
```cpp
bool negotiateProtocolVersion(uint16_t version = DEFAULT_PROTOCOL_VERSION)
```
- ***Function***
Sends the protocol version verification.
- ***Parameters***
    - version: The protocol version.
- ***Return Value***: Returns true if the verification is successful, and false otherwise.

---

### ***Get the Version of Elite CS Controller***
```cpp
VersionInfo getControllerVersion()
```
- ***Function***
Gets the version of Elite CS Controller.
- ***Return Value***: The version of Elite CS Controller.

---

### ***Configure the Output Subscription Recipe***
```cpp
RtsiRecipeSharedPtr setupOutputRecipe(const std::vector<std::string>& recipe_list, double frequency = 250)
```
- ***Function***
Configures the output subscription recipe.
- ***Parameters***
    - recipe_list: The recipe string. For specific content, refer to the Elite official document "RTSI User Manual".
    - frequency: The update frequency.
- ***Return Value***: The output subscription recipe. If it is nullptr, it means the subscription fails.

---

### ***Configure the Input Subscription Recipe***
```cpp
RtsiRecipeSharedPtr setupInputRecipe(const std::vector<std::string>& recipe)
```
- ***Function***
Configures the input subscription recipe.
- ***Parameters***
    - recipe: The recipe string. For specific content, refer to the Elite official document "RTSI User Manual".
- ***Return Value***: The input subscription recipe. If it is nullptr, it means the subscription fails.

---

### ***Start Data Synchronization***
```cpp
bool start()
```
- ***Function***
Starts data synchronization.
- ***Return Value***: Returns true if successful, and false if failed.

---

### ***Pause Data Synchronization***
```cpp
bool pause()
```
- ***Function***
Pauses data synchronization.
- ***Return Value***: Returns true if successful, and false if failed.

---

### ***Send the Input Subscription Recipe***
```cpp
void send(RtsiRecipeSharedPtr& recipe)
```
- ***Function***
Sends the input subscription recipe and sets the data for the robot.
- ***Parameters***
    - recipe: The input subscription recipe.

---

### Receive Output Subscription
```cpp
int receiveData(std::vector<RtsiRecipeSharedPtr>& recipes, bool read_newest = false)
```
- ***Function***
Receives the recipe data of the output subscription.
- ***Parameters***
    - recipes: The list of output subscription recipes. Only one recipe is received, and the data of the recipe in the list is updated. It is recommended that read_newest be false.
    - read_newest: Whether to receive the latest data packet. (There may be multiple data packets in the system cache.)
- ***Return Value***: The ID of the received recipe.

---

### Receive Output Subscription
```cpp
bool receiveData(RtsiRecipeSharedPtr recipe, bool read_newest = false)
```
- ***Function***
Receives the recipe data of the output subscription.
- ***Parameters***
    - recipe: The output subscription recipe. In the case of multiple recipes, if the received recipe is not the input recipe, the data of this recipe will not be updated.
    - read_newest: Whether to receive the latest data packet. (There may be multiple data packets in the system cache.)
- ***Return Value***: Returns true if the recipe is updated successfully.

---

### ***Connection Status***
```cpp
bool isConnected()
```
- ***Function***
Checks the connection status.
- ***Return Value***: Returns true if the connection is normal, and false otherwise.

---

### ***Synchronization Status***
```cpp
bool isStarted()
```
- ***Function***
Checks whether the robot data synchronization has been started.
- ***Return Value***: Returns true if yes, and false if no.

---

### ***Readable Status***
```cpp
bool isReadAvailable()
```
- ***Function***
Checks whether there is readable data. It is usually used to determine whether there is readable data in the buffer when receiving the robot status.
- ***Return Value***: Returns true if yes, and false if no.

---

# RtsiIOInterface Class

## Introduction
Inherits from the `RtsiClientInterface` class. This interface further encapsulates `RtsiClientInterface` and starts a thread internally to synchronize the robot data. Since this interface encapsulates many interfaces with the same nature, such as: getPayloadMass(), getTimestamp(), etc. For such interfaces, they will not be elaborated in the document for now. You can directly check the comments in the header file to obtain their functions.

## Header File
```cpp
#include <Elite/RtsiIOInterface.hpp>
```

## Constructor and Destructor

### ***Constructor***
```cpp
RtsiIOInterface::RtsiIOInterface(const std::string& output_recipe_file, const std::string& input_recipe_file, double frequency)
```
- ***Function***
Initializes the data and reads the two files `output_recipe_file` and `input_recipe_file` to obtain the subscription recipes. The format of the recipe file is:  
```
Subscription item 1
Subscription item 2
```
- ***Parameters***
    - output_recipe_file: The path of the output recipe file.
    - input_recipe_file: The path of the input recipe file.
    - frequency: The data synchronization frequency.

---

### ***Destructor***
```cpp
RtsiIOInterface::~RtsiIOInterface()
```
- ***Function***
Disconnects the socket, ends the data synchronization thread, and releases resources.

## Interfaces

### ***Connection***
```cpp
bool connect(const std::string& ip)
```
- ***Function***
Connects to the robot's RTSI port, performs version verification, obtains the robot controller version information, configures the input and output subscription recipes, and starts the data synchronization thread.
- ***Parameters***
    - ip: The IP address of the robot.
- ***Return Value***: Returns true if successful, and false if failed.

---

### ***Disconnection***
```cpp
void disconnect()
```
- ***Function***
Disconnects the socket with the robot and ends the data synchronization thread.

### Get the Value of the Output Recipe
```cpp
template<typename T>
bool getRecipeValue(const std::string& name, T& out_value);
```
- ***Function***
Gets the value of the specified subscription item in the output recipe.
- ***Parameters***
    - name: The name of the subscription item.
    - out_value: The output value. Note that the type of this value needs to be consistent with the type in the RTSI document.
- ***Return Value***: Returns true if the acquisition is successful, and false if failed.

---

### Set the Value of the Output Recipe
```cpp
template <typename T>
bool setInputRecipeValue(const std::string& name, const T& value)
```
- ***Function***
Sets the value of the specified subscription item in the input recipe.
- ***Parameters***
    - name: The name of the subscription item.
    - out_value: The input value. Note that the type of this value needs to be consistent with the type in the RTSI document.
- ***Return Value***: Returns true if the setting is successful, and false if failed.

---

# RtsiRecipe Class

## Introduction
This interface provides some basic operations for the RTSI recipe. The RtsiRecipe class can only be obtained by the two interfaces `RtsiClientInterface::setupOutputRecipe()` and `RtsiClientInterface::setupInputRecipe()`.

## Header File
```cpp
#include <Elite/RtsiRecipe.hpp>
```

## Interfaces

### ***Get Value***
```cpp
template<typename T>
bool getValue(const std::string& name, T& out_value)
```
- ***Function***
Gets the value of the subscription item in the recipe.
- ***Parameters***
    - name: The name of the subscription item.
    - out_value: The output value of the subscription item. Note that the type of this value needs to be consistent with the type in the RTSI document.
- ***Return Value***: Returns true if the acquisition is successful, and false if failed.

---

### ***Set Value***
```cpp
template<typename T>
bool setValue(const std::string& name, const T& value)
```
- ***Function***
Sets the value of the subscription item in the recipe.
- ***Parameters***
    - name: The name of the subscription item.
    - out_value: The setting value of the subscription item. Note that the type of this value needs to be consistent with the type in the RTSI document.
- ***Return Value***: Returns true if the setting is successful, and false if failed.

---

### Get the Recipe
```cpp
const std::vector<std::string>& getRecipe()
```
- ***Function***
Gets the list of the names of the subscription items in the recipe.
- ***Return Value***: The list of the names of the subscription items in the recipe.

---

### Get the Recipe ID
```cpp
int getID()
```
- ***Function***
Gets the recipe ID.
- ***Return Value***: The recipe ID.

---