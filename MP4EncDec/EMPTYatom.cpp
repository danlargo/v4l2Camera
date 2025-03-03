#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseEMPTYatom( std::ifstream &file, struct atom_t atom )
{
    std::string padding = calcPadding(m_depth);

    unsigned int len = atom.size;
    char * buffer;

    std::cout   << padding << std::hex << "[\033[1;32m" 
                << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)atom.orig[0] << " " 
                << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)atom.orig[1] << " " 
                << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)atom.orig[2] << " " 
                << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)atom.orig[3] << " "
                << std::dec << "\033[0m], len <" << atom.size << " bytes>";

    // process the atom data
    //
    //      Payload: Usually a single data atom (though some keys like trkn have special formats):
    //          Size (4 bytes): Size of the data atom.
    //          Type (4 bytes): 0x64617461 (data).
    //
    //      Version (1 byte): Typically 0x00.
    //      Flags (3 bytes): Indicates data type:
    //          0x000000 – Binary/undefined data.
    //          0x000001 – UTF-8 text (most common).
    //          0x000015 – UTF-16 text (less common).
    //          0x00000D – Integer (e.g., for trkn or disk).
    //
    //      Reserved (4 bytes): Usually 0x00000000.
    //
    //      Data (variable length): The actual value (e.g., string, integer), length = Size - 16.

    // get the payload
    unsigned int p_size;
    char p_type[5]; p_type[4] = 0;
    file.read( (char *)&p_size, 4 );
    file.read( (char *)&p_type, 4 );
    len -= 8;

    std::cout << " [\033[1;33m" << p_type << "\033[0m], ";

    unsigned char ver;
    unsigned char flags[3];
    file.read( (char *)&ver, 1 );
    file.read( (char *)&flags, 3 );
    len -= 4;

    std::string fstr = "";
    if( flags[2] == 0x00 ) fstr = "binary";
    if( flags[2] == 0x01 ) fstr = "text/8";
    if( flags[2] == 0x15 ) fstr = "text/16";
    if( flags[2] == 0x17 ) fstr = "jpeg";
    if( flags[2] == 0x0D ) fstr = "integer";

    std::cout << "v"    << (int)ver << ", flags 0x" 
                        << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)flags[0]
                        << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)flags[1]
                        << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)flags[2] << std::dec
                        << " - " << fstr;

    unsigned int reserved;
    file.read( (char *)&reserved, 4 );
    len -= 4;

    // get any remaining data
    unsigned int real_size = swapOrder(p_size);
    if( real_size > 16 )
    {
        buffer = new char[real_size - 16];
        file.read( buffer, real_size - 16 );
        len -= real_size - 16;

        // display the data
        std::cout << ", [\033[1;33m";

        if( flags[2] == 0x00 ) // binary
        {
            for( int i = 0; i < real_size - 16; i++ )
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i] << std::dec << " ";
            }
        }
        else if( flags[2] == 0x01 ) // text/8
        {
            for( int i = 0; i < real_size - 16; i++ ) std::cout << buffer[i];
        }
        else // default, just display as binary
        {
            for( int i = 0; i < real_size - 16; i++ )
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i] << std::dec << " ";
            }
        }

        std::cout << "\033[0m]" << std::endl;

        delete [] buffer;
    }

}