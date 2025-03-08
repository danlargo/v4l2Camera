#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseVMHDatom( std::ifstream &file, unsigned long len )
{
    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">,";

    len -= 4;

    // The vmhd atom has the following structure:
    //
    //  Version (1 byte): Typically 0x00. Indicates the version of the vmhd format (no other versions are common).
    //
    //  Flags (3 bytes): Usually 0x000001. The least significant bit (0x01) is often set to indicate the atom is valid, though the spec doesn’t define specific flags beyond reserving the field.
    //
    //  Graphics Mode (2 bytes): A 16-bit unsigned integer specifying the video compositing mode:
    //      0x0000: Copy mode (default, direct rendering).
    //      0x0040: Blend mode (with transparency, rare in MP4).
    //      Other values (e.g., 0x0024 for dither copy) are QuickTime-specific and uncommon in modern MP4s.
    //
    //  Opcolor (6 bytes): Three 16-bit unsigned integers (red, green, blue) defining the operation color for the graphics mode:
    //      Each value ranges from 0 (0x0000) to 65535 (0xFFFF).
    //      Typically 0x0000 0000 0000 (black) for copy mode, ignored unless Graphics Mode requires it.

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
                << ">" << std::dec << std::endl;

}