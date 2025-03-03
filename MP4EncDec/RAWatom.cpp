#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseRAWatom(  std::ifstream &file, unsigned long len )
{
    // Display this data as raw binary hex
    char * buffer;
    buffer = new char[len];
    file.read( buffer, len );
    
    if( len == 0 )
    {
        std::cout << "[0 bytes]" << std::endl;
    } else {
        // display the entire len
        std::cout << std::dec << len << " bytes : (binary) ";

        // display the raw data following
        for( int i = 0; i < len; i++ )
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i] << " ";
        }

        // add a blank line
        std::cout << std::endl;

        // delete first part
        delete [] buffer;
    }

}