#ifndef ___ELITE_CONTROLLER_LOG_HPP__
#define ___ELITE_CONTROLLER_LOG_HPP__

#include <string>
#include <functional>

namespace ELITE {

class ControllerLog {
private:
public:
    /**
     * @brief Download system log from robot
     *
     * @param robot_ip Robot ip address
     * @param password Robot ssh password
     * @param path Save path
     * @param progress_cb Download progress callback function.
     *      f_z: File size.
     *      r_z: Downloaded size.
     *      err: Error information (nullptr when there is no error)
     * @return true success
     * @return false fail
     * @note **Ensure that the sshpass and scp commands are installed in your
     * system.**
     */
    static bool downloadSystemLog(
        const std::string &robot_ip, const std::string &password,
        const std::string &path,
        std::function<void(int f_z, int r_z, const char *err)> progress_cb);
    ControllerLog() {}
    ~ControllerLog() {}
};

} // namespace ELITE

#endif
