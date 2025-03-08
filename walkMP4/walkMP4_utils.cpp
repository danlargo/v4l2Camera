#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <algorithm>

#include "walkMP4.h"

// String Utilities
//

std::string toUpper(std::string str) 
{
    std::string result = "";
    for( unsigned int i = 0; i < str.length(); i++ ) { result += std::toupper(str[i]); }
    return result;
}

std::string toLower(std::string str) 
{
    std::string result = "";
    for( unsigned int i = 0; i < str.length(); i++ ) { result += std::tolower(str[i]); }
    return result;
}

// Trim from start (in place)
void ltrim(std::string &s) 
{
    bool done = false;

    while( !done )
    {
        if( s[0] == ' ' ) s.erase(0, 1);
        else if( s[0] == '\t' ) s.erase(0, 1);      // tab
        else if( s[0] == '\n' ) s.erase(0, 1);      // new line
        else if( s[0] == '\r' ) s.erase(0, 1);      // carriage return
        else if( s[0] == '\v' ) s.erase(0, 1);      // vertical tab
        else if( s[0] == '\f' ) s.erase(0, 1);      // form feed
        else break;
    }
}

// Trim from end (in place)
void rtrim(std::string &s) 
{
    bool done = false;

    while( !done )
    {
        int i = s.length()-1;
        if( s[i] == ' ' ) s.erase(i, 1);
        else if( s[i] == '\t' ) s.erase(i, 1);
        else if( s[i] == '\n' ) s.erase(i, 1);
        else if( s[i] == '\r' ) s.erase(i, 1);
        else if( s[i] == '\v' ) s.erase(i, 1);
        else if( s[i] == '\f' ) s.erase(i, 1);
        else break;
    }
}

// Trim from both ends (in place)
void trim(std::string &s) 
{
    ltrim(s);
    rtrim(s);
}


// Out[put Version Information

std::string getVersionString() 
{ 
    std::string ver = "v"; 
    ver += std::to_string(s_majorVersion); ver += "."; 
    ver += std::to_string(s_minorVersion); ver += "."; 
    ver += std::to_string(s_revision);  
    return ver;
}


// Endian Functions
//
unsigned int swapEndian( unsigned int in )
{
    return (in >> 24) | ((in << 8) & 0x00FF0000) | ((in >> 8) & 0x0000FF00) | (in << 24);
}

int swapEndian( int in )
{
    return (in >> 24) | ((in << 8) & 0x00FF0000) | ((in >> 8) & 0x0000FF00) | (in << 24);
}

unsigned short swapEndian( unsigned short in )
{
    return (in >> 8) | (in << 8);
}

unsigned long swapEndian( unsigned long in )
{
    return ((in >> 56) & 0x00000000000000FF) |
           ((in >> 40) & 0x000000000000FF00) |
           ((in >> 24) & 0x0000000000FF0000) |
           ((in >> 8)  & 0x00000000FF000000) |
           ((in << 8)  & 0x000000FF00000000) |
           ((in << 24) & 0x0000FF0000000000) |
           ((in << 40) & 0x00FF000000000000) |
           ((in << 56) & 0xFF00000000000000);
}