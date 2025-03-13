#ifndef __ELITE__REMOTE_UPGRADE_HPP__
#define __ELITE__REMOTE_UPGRADE_HPP__


#include <string>

namespace ELITE
{

namespace UPGRADE
{
#if defined(__linux) || defined(linux) || defined(__linux__)
/**
 * @brief Upgrade the robot control software
 * 
 * @param ip Robot ip
 * @param file Upgrade file
 * @param password Robot controller ssh password
 * @return true success
 * @return false fail
 * @note 
 *      1. Windows not support yet.
 *      2. Ensure that the ssh and scp commands are installed in your system.
 */
bool upgradeControlSoftware(std::string ip, std::string file, std::string password);
#endif

}
}

#endif