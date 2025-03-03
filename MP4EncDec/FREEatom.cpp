#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseFREEatom(  std::ifstream &file, unsigned long len )
{
    unsigned long grid_size = 32;

    // this is free data atom
    char * buffer;

    // read the entire buffer, the free section isn't that long
    buffer = new char[len];
    file.read( buffer, len );
    
    // display the len
    std::cout << std::dec << len << " bytes : (pad) [\033[1;33m";

    // now dump it as ascii...
    for( int i = 0; i < len; i++ )
    {
        if( (buffer[i] >= 0x20) && (buffer[i] <= 0x7e) ) std::cout << buffer[i];
        else std::cout << "";
    }

    // display the raw data
    //std::cout << std::endl;
    //for( int i = 0; i < len; i++ )
    //{
    //    if( (i% grid_size == 0) ) std::cout << std::endl << "          " << padding;
    //    std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i] << " ";
    //}

    delete [] buffer;

    // add a blank line
    std::cout << "\033[0m]"<< std::dec << std::endl;

}