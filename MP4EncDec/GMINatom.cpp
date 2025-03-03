#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseGMINatom( std::ifstream &file, unsigned long len )
{
    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">,";

    len -= 4;

    // The GMIN atom has the following structure:
    //
    //  Version (1 byte): Usually 0x00.
    //
    //  Flags (3 bytes): Usually 0x000000.
    //
    //  Graphics Mode (2 bytes): Same as vmhd (e.g., 0x0000 for copy).
    //
    //  Opcolor (6 bytes): Three 16-bit RGB values (e.g., 0x0000 0000 0000).
    //
    //  Balance (2 bytes): Same as smhd (e.g., 0x0000 for centered).
    //
    //  Reserved (2 bytes): 0x0000.


    unsigned short mode;
    file.read( (char *)&mode, 2 );
    len -= 2;

    std::string modeStr = "";
    switch( swapShort(mode) )
    {
        case 0x0000: modeStr = "direct-render"; break;
        case 0x0024: modeStr = "dither-copy"; break;
        case 0x0040: modeStr = "alpha-blend"; break;
        default: modeStr = "custom";
    }

    unsigned short color[3];
    file.read( (char *)&color, 6 );
    len -= 6;

    std::cout   << " mode <0x" << std::hex << swapShort(mode) 
                << " " << modeStr << ">, colors <r: 0x" << swapShort(color[0])
                << " g: 0x" << swapShort(color[1])
                << " b: 0x" << swapShort(color[2])
                << ">" << std::dec;

    unsigned char balance[2];
    file.read( (char *)&balance, 2 );
    len -= 2;

    unsigned short reserved;
    file.read( (char *)&reserved, 2 );
    len -= 2;

    std::cout << ", sound balance <left " << (int)balance[0] << ":" << (int)balance[1] << " right>" << std::endl;
}