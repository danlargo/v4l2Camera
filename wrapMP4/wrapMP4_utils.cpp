#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <algorithm>

#include "wrapMP4.h"

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


// MP4 language conversion
//
std::string decode_lang(uint16_t lang) 
{
    // Extract 5-bit values
    uint8_t char1 = (lang >> 10) & 0x1F;  // Bits 14–10
    uint8_t char2 = (lang >> 5) & 0x1F;   // Bits 9–5
    uint8_t char3 = lang & 0x1F;          // Bits 4–0

    // Convert to ASCII by adding 0x60
    std::string result;
    result += (char1 ? char1 + 0x60 : ' ');
    result += (char2 ? char2 + 0x60 : ' ');
    result += (char3 ? char3 + 0x60 : ' ');

    return result;
}

uint16_t encode_lang(std::string lang) 
{
    // Convert ASCII to 5-bit values by subtracting 0x60
    uint8_t char1 = lang[0] - 0x60;
    uint8_t char2 = lang[1] - 0x60;
    uint8_t char3 = lang[2] - 0x60;

    // Combine into 16-bit value
    return (char1 << 10) | (char2 << 5) | char3;
}


std::string mp4ToString( unsigned int s_time )
{
    std::string ret = "";

    s_time = swapEndian(s_time);

    if( s_time <= 0 ) return "-";

    #define MP4_EPOCH_OFFSET 2082844800ULL  // 66 years, accounting for leap years

    time_t tmp_time = (time_t)(s_time - MP4_EPOCH_OFFSET);

    struct tm *time_info;
    char buffer[80];

    time_info = gmtime(&tmp_time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);

    ret = buffer;

    return ret;
}


unsigned int mp4ToInt32( std::string fmt_str )
{
    time_t now = time(0);
    time_t t = now;

    // createt a time value based on the input string
    if( fmt_str.length() > 0 )
    {
        struct tm tm;
        strptime(fmt_str.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
        t = mktime(&tm);
    }

    return (unsigned int)(t + 2082844800);
}

unsigned int buildINT16_2( unsigned short a, unsigned short b )
{
    unsigned int ret = 0;
    ret = (swapEndian(a) << 16) | swapEndian(b);
    return ret;
}