#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <chrono>

#include "defines.h"

#ifdef __linux__
    #include <unistd.h>
    #include "linux/linuxcamera.h"
#elif __APPLE__
    #include <unistd.h>
    #include "macos/accamera.h"
    #include "macos/v4l2cam_defs.h"
#elif _WIN32
    #include "wincamera.h"
#endif

void outln( std::string line )
{
    std::cout << "   " << line << std::endl;
}

void outerr( std::string line )
{
    // always print the error messages
    std::cerr << "[\x1b[1;31mwarning\x1b[0m] " << line << std::endl;
}

void outinfo( std::string line )
{
    std::cerr << "[\x1b[1;33minfo\x1b[0m] " << line << std::endl;
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

std::string makeHexString( unsigned char * buf, int len, bool makeCaps )
{
    std::stringstream tmp;

    int bufOff = 0;

    while( len > 0 )
    {
        if( makeCaps ) tmp << std::uppercase;
        tmp << std::setfill('0') << std::setw(2) << std::hex << (int)buf[bufOff++];
        len--;
    }

    return tmp.str();
}