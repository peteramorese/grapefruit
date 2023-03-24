#pragma once

#include <iostream>
#include <iostream>

#include "Logging.h"

#define TP_DEBUG_TOOLS

#ifdef TP_DEBUG_TOOLS
    #define PAUSE std::cin.get();
    #define PAUSE_IF(condition, msg) if (condition) {LOG(msg); std::cin.get();};
#else
    #define PAUSE 
#endif

