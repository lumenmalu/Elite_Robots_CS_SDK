#include <gtest/gtest.h>
#include <string>
#include <cstdint>
#include <chrono>

#include "Common/SshUtils.hpp"
#include "Elite/Logger.hpp"

using namespace ELITE;

static std::string s_user;
static std::string s_ip;
static std::string s_ssh_pw;

TEST(sshUtilsTest, ssh_utils_test) {
    SSH_UTILS::executeCommand(s_ip, s_user, s_ssh_pw, "");
    std::string cmd_output = 
        SSH_UTILS::executeCommand(
            s_ip, 
            s_user, 
            s_ssh_pw, 
            "export ELITE_TEST_SSH_ENV=ABCD\n"
            "printenv ELITE_TEST_SSH_ENV");
    EXPECT_EQ(cmd_output, "ABCD\n");
}


int main(int argc, char** argv) {
    if (argc < 4 || argv[1] == nullptr || argv[2] == nullptr || argv[3] == nullptr) {
        std::cout << "cmd format:\n  SshUtilsTest <user> <ip> <password>" << std::endl;
        return 1;
    }
    ELITE::setLogLevel(ELITE::LogLevel::ELI_DEBUG);
    s_user = argv[1];
    s_ip = argv[2];
    s_ssh_pw = argv[3];
    
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
