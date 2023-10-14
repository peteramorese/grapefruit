#pragma once

#include <iostream>

#define GF_LOG_COLOR
#define GF_ASSERTS

#ifdef GF_LOG_COLOR
    #define LOG(msg) std::cout << "\033[1;36m > ("<< __func__ << "): \033[0;37m" << msg << "\033[0m \n"
    #define PRINT(msg) std::cout << "\033[0;37m" << msg << "\033[0m \n"
    #define PRINT_NAMED(name, msg) std::cout << "\033[1;32m "<< name << ": \033[0;37m" << msg << "\033[0m \n"
    #define ERROR(msg) std::cout << "\033[1;31m > ERROR ("<< __func__ << "): \033[0;41m" << msg << "\033[0m \n"
    #define WARN(msg) std::cout << "\033[1;33m > WARNING ("<< __func__ << "): \033[0;33m" << msg << "\033[0m \n"
    #define NEW_LINE std::cout<<"\n"
#elif GF_LOG_NO_COLOR
    #define LOG(msg) std::cout << " > ("<< __func__ << "): " << msg << "\033[0m \n"
    #define PRINT(msg) std::cout <<  msg << "\n"
    #define PRINT_NAMED(name, msg) std::cout << " "<< name << ": " << msg << "\n"
    #define ERROR(msg) std::cout << " > ERROR ("<< __func__ << "): " << msg << "\n"
    #define WARN(msg) std::cout << " > WARNING ("<< __func__ << "): " << msg << "\n"
    #define NEW_LINE std::cout<<"\n"
#else
    #define LOG(msg) 
    #define PRINT(msg) 
    #define PRINT_NAMED(name, msg) 
    #define NEW_LINE 
#endif

#ifdef GF_ASSERTS
    #define ASSERT(condition, msg) {if (!(condition)) {ERROR("[Assert fail] " << msg); exit(1);}}
#endif

