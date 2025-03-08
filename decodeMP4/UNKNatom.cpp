#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseUNKNatom(  std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    unsigned int disp_len = 32;
    unsigned int char_len = 80;

    // this is free data atom
    char * buffer;
    buffer = new char[len];
    file.read( buffer, len );
    
    if( len == 0 )
    {
        std::cout << "[0 bytes]" << std::endl;
    } else {
        // display the entire len
        std::cout << std::dec << "\033[1;31m??\033[0m " << len << " bytes : \033[1;33m";

        // now dump first part as ascii...
        int char_count = 0;
        for( int i = 0; i < len; i++ )
        {
            if( (buffer[i] >= 0x20) && (buffer[i] <= 0x7e) ) 
            {
                std::cout << buffer[i];
                char_count++;
            }
            if( char_count >= char_len ) break;
        }

        std::cout << "\033[0m" << std::endl;
        std::cout << padding << "  raw data : ";

        // display the raw data following but only the first 32 bytes
        for( int i = 0; i < disp_len; i++ )
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i] << " ";
        }

        // add a blank line
        std::cout << std::endl;
    }

    // delete first part
    delete [] buffer;

    m_depth--;
}