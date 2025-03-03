#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseSTRNGatom(  std::ifstream &file, unsigned long len )
{
    unsigned long disp_len = 24;

    // this is free data atom
    char * buffer;

    // read/display the first part of the buffer
    if( len < disp_len ) disp_len = len;

    buffer = new char[disp_len];
    file.read( buffer, disp_len );
    
    if( len == 0 )
    {
        std::cout << "[0 bytes]" << std::endl;
    } else {
        // display the entire len
        std::cout << std::dec << len << " bytes : (string) \033[1;33m";

        // now dump first part as ascii...
        for( int i = 0; i < disp_len; i++ )
        {
            if( (buffer[i] >= 0x20) && (buffer[i] <= 0x7e) ) std::cout << buffer[i];
            else std::cout << ".";
        }

        std::cout << "\033[0m : ";

        // display the raw data following
        for( int i = 0; i < disp_len; i++ )
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i] << " ";
        }

        // add a blank line
        std::cout << std::endl;

        // delete first part
        delete [] buffer;

        // read the rest of the buffer to progress in the file
        if( len > disp_len )
        {
            buffer = new char[len - disp_len];
            file.read( buffer, len - disp_len );

            delete [] buffer;
        }
    }

}