#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseTCMIatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding( m_depth );

    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">,";

    len -= 4;

    //  The TMCI atom has the following structure:
    //
    //  Version (1 byte): Usually 0x00.
    //
    //  Flags (3 bytes): Usually 0x000000.
    //
    //  Text Font (2 bytes): 0x0015 (font ID, 21 = Helvetica).
    //
    //  Text Face (2 bytes): 0x0000 (normal face, no bold/italic).
    //
    //  Text Size (2 bytes): 0x000A (10-point font size).
    //
    //  Reserved (2 bytes): 0x0000.
    //
    //  Text Color (6 bytes): 0x0000 0000 0000 (RGB, black).
    //
    //  Background Color (6 bytes): 0xFFFF FFFF FFFF (RGB, white).
    //  
    //  Font Name (variable, 13 bytes here): Pascal string (0x09 48 65 6C 76 65 74 69 63 61 = length 9, "Helvetica").

    unsigned short font;
    file.read( (char *)&font, 2 );
    len -= 2;

    std::string fontStr = "";
    switch( swapShort(font) )
    {
        case 0x0000: fontStr = "unknown"; break;
        case 0x0001: fontStr = "Times"; break;
        case 0x0002: fontStr = "Times New Roman"; break;
        case 0x0003: fontStr = "Courier"; break;
        case 0x0004: fontStr = "Symbol"; break;
        case 0x0005: fontStr = "ZapfDingbats"; break;
        case 0x0006: fontStr = "Bookman"; break;
        case 0x0007: fontStr = "Helvetica"; break;
        case 0x0008: fontStr = "Helvetica Bold"; break;
        case 0x0009: fontStr = "Helvetica Italic"; break;
        case 0x000A: fontStr = "Helvetica Bold Italic"; break;
        case 0x000B: fontStr = "Arial"; break;
        case 0x000C: fontStr = "Arial Bold"; break;
        case 0x000D: fontStr = "Arial Italic"; break;
        case 0x000E: fontStr = "Arial Bold Italic"; break;
        case 0x000F: fontStr = "Palatino"; break;
        case 0x0010: fontStr = "Palatino Bold"; break;
        case 0x0011: fontStr = "Palatino Italic"; break;
        case 0x0012: fontStr = "Palatino Bold Italic"; break;
        case 0x0013: fontStr = "Avant Garde"; break;
        case 0x0014: fontStr = "Avant Garde Bold"; break;
        case 0x0015: fontStr = "Helvetica"; break;
        default: fontStr = "custom";
    }

    unsigned short face;
    file.read( (char *)&face, 2 );
    len -= 2;

    std::string faceStr = "";
    if( face == 0x0000 ) faceStr = "normal";
    else
    {
        if( face & 0x0001 ) faceStr += "bold ";
        if( face & 0x0002 ) faceStr += "italic ";
    }

    unsigned short size;
    file.read( (char *)&size, 2 );
    len -= 2;

    unsigned short reserved;
    file.read( (char *)&reserved, 2 );
    len -= 2;

    unsigned short color[3];
    file.read( (char *)&color, 6 );
    len -= 6;

    unsigned short bcolor[3];
    file.read( (char *)&bcolor, 6 );
    len -= 6;

    unsigned char font_name_len;
    file.read( (char *)&font_name_len, 1 );
    len -= 1;

    unsigned char * font_name = new unsigned char[font_name_len + 1];
    file.read( (char *)font_name, font_name_len );
    font_name[font_name_len] = 0;
    len -= font_name_len;

    std::cout << " font <" << swapShort(font) << ", \033[1;36m" << fontStr << "\033[0m>, face <" << swapShort(face) << ", \033[1;36m" << faceStr << "\033[0m>, size <\033[1;36m" << swapShort(size) << " pts\033[0m>," << std::endl;
    std::cout << padding << std::hex << "  colors <r: 0x" << swapShort(color[0]) << " g: 0x" << swapShort(color[1]) << " b: 0x" << swapShort(color[2]) << ">,"
                << " bcolors <r: 0x" << swapShort(bcolor[0]) << " g: 0x" << swapShort(bcolor[1]) << " b: 0x" << swapShort(bcolor[2]) << ">"
                << ", font name <" << font_name << ">" << std::dec << std::endl;

    delete [] font_name;

    m_depth--;

}