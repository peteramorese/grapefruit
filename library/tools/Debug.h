#pragma once

#include <iostream>
#include <iostream>

#define TP_DEBUG_TOOLS

#ifdef TP_DEBUG_TOOLS
    #define PAUSE std::cin.get();
#else
    #define PAUSE 
#endif

