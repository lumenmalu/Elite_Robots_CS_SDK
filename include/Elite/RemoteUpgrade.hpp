#ifndef __ELITE__REMOTE_UPGRADE_HPP__
#define __ELITE__REMOTE_UPGRADE_HPP__


#include <string>

namespace ELITE
{

namespace UPGRADE
{

/**
 * @brief Upgrade the robot control software
 * 
 * @param ip Robot ip
 * @param file Upgrade file
 * @param password robot controller ssh password
 * @return true success
 * @return false fail
 * @note Windows not support yet
 */
bool upgradeControlSoftware(std::string ip, std::string file, std::string password);


}
}

#endif