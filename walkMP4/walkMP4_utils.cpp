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

// Trim from start (in place)
void ltrim(std::string &s) 
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// Trim from end (in place)
void rtrim(std::string &s) 
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
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