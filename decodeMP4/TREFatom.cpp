#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseTREFatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    // Sequence of ...
    //
    //  Size (4 bytes): Size of the child atom.
    //
    //  Type (4 bytes): A 4-character code indicating the reference type.
    //
    //  Payload: One or more 4-byte track IDs (32-bit unsigned integers) identifying the referenced tracks.
    //
    std::cout << std::endl;
    while( len > 0 )
    {
        unsigned int size;
        char type[5]; type[4] = 0;
        file.read( (char *)&size, 4 );
        file.read( (char *)&type, 4 );
        len -= 8;

        // display it
        std::cout << padding << "[\033[1;32m" << toUpper(type) << "\033[0m] " << swapOrder(size)-8 << " bytes, ids [\033[1;35m";

        // display the data
        int real_size = swapOrder(size)-8;

        for( int i=0; i< real_size; i+=4 )
        {
            unsigned int track_id;
            file.read( (char *)&track_id, 4 );
            len -= 4;

            std::cout << swapOrder(track_id);
        }


        std::cout << "\033[0m]" << std::endl;
    }
    m_depth--;
}