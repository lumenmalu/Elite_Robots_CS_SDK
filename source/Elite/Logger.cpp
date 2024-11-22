#include "Logger.hpp"

namespace ELITE{


static Logger s_logger;

Logger& getLogger() {
    return s_logger;
}

}
