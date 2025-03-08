#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <algorithm>

#include "decodeMP4.h"

struct atom_t getATOMhdr( std::ifstream &file )
{
    struct atom_t atom;

    unsigned int size;
    file.read( (char *)&size, 4 );
    atom.bytes_read = 4;

    unsigned int real_size = swapOrder(size);

    atom.orig[4] = 0;
    file.read( (char *)&atom.orig, 4 );
    atom.bytes_read += 4;

    std::string tmp = (char *)atom.orig;
    atom.tag = toUpper(tmp);

    if( real_size == 1 )
    {
        // read the extended size
        unsigned long ext_size;
        file.read( (char *)&ext_size, 8 );
        atom.bytes_read += 8;

        atom.size = swapLong(ext_size) - 16;

    } else atom.size = real_size - 8;

    return atom;
}


void parseTIMEhdr( std::ifstream &file, unsigned char version, std::string padding )
{
    //     Creation Time:
    //          Version 0: 4 bytes (32-bit unsigned integer).
    //          Version 1: 8 bytes (64-bit unsigned integer).
    //              Represents seconds since midnight, January 1, 1904 (UTC), the epoch used by QuickTime and MP4.
    //
    //      Modification Time:
    //          Version 0: 4 bytes.
    //          Version 1: 8 bytes.
    //              Same epoch as creation time.

    struct atom_time0_t timev0, timev1;

    if( version == 0 )
    {
        file.read( (char *)&timev0, 8 );

        #define MP4_EPOCH_OFFSET 2082844800ULL  // 66 years, accounting for leap years

        time_t tmp_time = (time_t)(swapOrder(timev0.created) - MP4_EPOCH_OFFSET);
        struct tm *time_info;
        char buffer[80];
        if( swapOrder(timev0.created) == 0 )
        {
            std::cout << padding << "created < - ";
        } else {
            time_info = gmtime(&tmp_time);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
            std::cout << padding << "created <\033[1;33m" << buffer << "\033[0m";
        }

        tmp_time = (time_t)(swapOrder(timev0.modified) - MP4_EPOCH_OFFSET);
        if( swapOrder(timev0.modified) == 0 )
        {
            std::cout << ">  modified < - ";
        } else {
            time_info = gmtime(&tmp_time);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
            std::cout << ">  modified <\033[1;33m" << buffer << "\033[0m";
        }
        std::cout << ">  v0 (" << sizeof(timev0) << "b)";

    } else
    {
        file.read( (char *)&timev1, 16 );

        time_t tmp_time = (time_t)(timev1.created);
        struct tm *time_info = gmtime(&tmp_time);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
        std::cout << "created <\033[1;33m" << buffer << "\033[0m";
        tmp_time = (time_t)timev1.modified;
        time_info = gmtime(&tmp_time);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
        std::cout << ">  modified <\033[1;33m" << buffer << "\033[0m";
        std::cout << ">  v1 (" << sizeof(timev1) << "b)";
    }
}


// Utilities
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


std::string getVersionString() 
{ 
    std::string ver = "v"; 
    ver += std::to_string(s_majorVersion); ver += "."; 
    ver += std::to_string(s_minorVersion); ver += "."; 
    ver += std::to_string(s_revision);  
    return ver;
}

unsigned int swapOrder( unsigned int in )
{
    return (in >> 24) | ((in << 8) & 0x00FF0000) | ((in >> 8) & 0x0000FF00) | (in << 24);
}

unsigned short swapShort( unsigned short in )
{
    return (in >> 8) | (in << 8);
}

unsigned long swapLong( unsigned long in )
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

std::string decode_lang(uint16_t lang) {
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

std::string calcPadding( int depth )
{
    std::string ret;
    // offset for the atom count
    ret += "   ";
    for( int i = 0; i < depth; i++ ) ret += "     ";

    return ret;
}