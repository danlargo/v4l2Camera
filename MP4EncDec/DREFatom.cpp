#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseDREFatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">,";

    len -= 4;

    // now parse the rest of the header
    //
    //  Version (1 byte): Typically 0x00.
    //  Flags (3 bytes): Usually 0x000000 (no flags defined).
    //
    //  Payload
    //
    //      Entry Count (4 bytes): A 32-bit unsigned integer specifying the number of data reference entries that follow.
    //  
    //      Data Reference Entries (variable length, Entry Count × entry size):
    //
    //          Each entry is itself an atom describing a data source.
    //
    //          Common entry types:
    //              url  (URL Reference):
    //                  Size (4 bytes): Size of the url  atom.
    //                  Type (4 bytes): 0x75726C20 (ASCII url , note the space).
    //                  Version (1 byte): 0x00.
    //                  Flags (3 bytes):
    //                      0x000001: Data is in the same file (self-contained, no URL string).
    //                      0x000000: External URL follows (rare for typical MP4s).
    //                  Data (variable, optional): UTF-8 URL string (only if Flags != 0x000001).
    //
    //              urn  (URN Reference):
    //                  Size (4 bytes).
    //                  Type (4 bytes): 0x75726E20 (ASCII urn ).
    //                  Version (1 byte): 0x00  
    //                  Flags (3 bytes): Similar to url .
    //                  Data (variable): Name (URN) + optional location (URL).
    //
    unsigned int count;
    file.read( (char *)&count, 4 );
    len -= 4;

    std::cout << " count <" << swapOrder(count) << ">, len <" << len << ">" << std::endl;

    while( len > 0 )
    {
        unsigned int size;
        char type[5]; type[4] = 0;
        file.read( (char *)&size, 4 );
        file.read( (char *)&type, 4 );
        len -= 8;

        // display it
        std::cout << padding << "[\033[1;32m" << toUpper(type) << "\033[0m] " << swapOrder(size)-12 << " bytes,";

        struct version_t sub_ver;
        file.read( (char *)&sub_ver , 4 );
        std::cout << "v" << (int)sub_ver.version << ", flags <" << std::hex << (int)sub_ver.flags[0] << (int)sub_ver.flags[1] << (int)sub_ver.flags[2] << std::dec;
        if( sub_ver.flags[2] == 1 ) std::cout << " (internal)";
        std::cout << ">";
        len -= 4;

        // display the data
        int real_size = swapOrder(size);
        if( real_size > 12 )
        {
            unsigned char * fld = new unsigned char[real_size - 12];
            file.read( (char *)fld, real_size - 12 );
            len -= real_size - 12;

            std::cout << ", data <";
            for( int i = 0; i < real_size - 12; i++ ) std::cout << fld[i];
            std::cout << ">";

            delete [] fld;
        }

        std::cout << std::endl;
    }

    m_depth--;

}