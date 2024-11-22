#ifndef __ELITE__DEFATULT_LOG_HPP__
#define __ELITE__DEFATULT_LOG_HPP__

#include "Log.hpp"
#include <iostream>

namespace ELITE
{

class DefaultLogHandler : public LogHandler
{
private:
    
public:
    DefaultLogHandler() = default;
    ~DefaultLogHandler() = default;

    void log(const char* file, int line, LogLevel level, const char* log) {
        switch (level) {
        case LogLevel::ELI_DEBUG:
            std::cout << "[DEBUG] " << file << ":" << line << ": " << log << std::endl;
            break;
        case LogLevel::ELI_INFO:
            std::cout << "[INFO] " << file << ":" << line << ": " << log << std::endl;
            break;
        case LogLevel::ELI_WARN:
            std::cout << "[WARN] " << file << ":" << line << ": " << log << std::endl;
            break;
        case LogLevel::ELI_ERROR:
            std::cout << "[ERROR] " << file << ":" << line << ": " << log << std::endl;
            break;
        case LogLevel::ELI_FATAL:
            std::cout << "[FATAL] " << file << ":" << line << ": " << log << std::endl;
            break;
        case LogLevel::ELI_NONE:
            std::cout << "[NONE] " << file << ":" << line << ": " << log << std::endl;
            break;
        default:
            break;
        }
    }

};


} // namespace ELITE




#endif
