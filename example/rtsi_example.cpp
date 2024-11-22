#include "Elite/RtsiIOInterface.hpp"
#include <memory>
#include <iostream>

using namespace ELITE;

// In a real-world example it would be better to get those values from command line parameters / a
// better configuration system such as Boost.Program_options
const std::string DEFAULT_ROBOT_IP = "192.168.51.244";

int main(int argc, char* argv[]) {
    // Parse the ip arguments if given
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

    if ((io_interface->getDigitalOutputBits() & 0x00000001)) {
        auto start_set_false = std::chrono::high_resolution_clock::now();
        io_interface->setStandardDigital(0, false);
        while (io_interface->getDigitalOutputBits() | 0x00000000) {
            ;
        }
        auto finish_set_false = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_set_false = finish_set_false - start_set_false;
        std::cout << "Setting low level cost time: " << elapsed_set_false.count() << std::endl;
    }

    auto start_set_true = std::chrono::high_resolution_clock::now();
    io_interface->setStandardDigital(0, true);

    while (!(io_interface->getDigitalOutputBits() & 0x00000001)) {
        ;
    }
    auto finish_set_true = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_set_true = finish_set_true - start_set_true;
    std::cout << "Setting high level cost time: " << elapsed_set_true.count() << std::endl;
    
    io_interface->disconnect();

    return 0;
}
