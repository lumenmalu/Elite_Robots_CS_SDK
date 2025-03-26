#ifndef __ELITE__REMOTE_UPGRADE_HPP__
#define __ELITE__REMOTE_UPGRADE_HPP__


#include <string>
#include <Elite/EliteOptions.hpp>

namespace ELITE
{

namespace UPGRADE
{
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
ELITE_EXPORT bool upgradeControlSoftware(std::string ip, std::string file, std::string password);

}
}

#endif