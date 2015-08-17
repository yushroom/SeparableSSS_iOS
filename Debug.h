//
//  Debug.h
//  SeparableSSS
//
//  Created by yushroom on 8/13/15.
//
//

#ifndef SeparableSSS_Debug_h
#define SeparableSSS_Debug_h

#include <cstdio>
#include <iostream>
#include <string>

class Debug
{
public:
    static void init()
    {
        
    }
    
    static void LogError(const std::string info)
    {
        //printf("[ERROR] %s\n", info);
        std::cout << "[Error] " << info << std::endl;
    }
    
    static void LogWarning(const char * info)
    {
        std::cout << "[Error] " << info << std::endl;
    }
    
    static void LogInfo(const char * info)
    {
        std::cout << "[Error] " << info << std::endl;
    }
    
private:
    Debug();
    ~Debug();
};


#endif
