#include <iostream>
#include <fstream>
#include <iomanip>

#include "decodeH264.h"

char * h264GrabFrame( std::ifstream &file )
{
    // read the entire file into a buffer
    file.seekg( 0, std::ios::end );
    int length = file.tellg();
    file.seekg( 0, std::ios::beg );

    char * buffer = new char[length];
    file.read( buffer, length );

    std::cout << "read " << length << " bytes" << std::endl << std::endl;

    // walk the buffer looking for the start of the next frame
    int i = 0;
    while( i < length )
    {
        // find the start of the next frame
        if( (buffer[i] == 0x00) && (buffer[i+1] == 0x00) && (buffer[i+2] == 0x00) && (buffer[i+3] == 0x01) )
        {
            switch( buffer[i+4] )
            {
                case 0x67: // SPS
                    std::cout << "found Sequence Parameter Set at offset : " << i << std::endl;
                    break;
                case 0x68: // PPS
                    std::cout << "found Picture Parameter Set at offset : " << i << std::endl;
                    break;
                case 0x65: // IDR
                    std::cout << "found Key-Frame at offset : " << i << std::endl;
                    break;
                case 0x61: // NDR
                    std::cout << "found B/P-Frame at offset : " << i << std::endl;
                    break;
            }
        }
        i++;
    }

    return buffer;
}