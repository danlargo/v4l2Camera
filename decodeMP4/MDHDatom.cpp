#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseMDHDatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << "> ";

    parseTIMEhdr( file, ver.version, "" );

    // now parse the rest of the header
    //
    //  Timescale (4 bytes): Units per second (e.g., 0x00003E80 = 16,000 for audio).
    //
    //  Duration (4 or 8 bytes): Length in timescale units.
    //
    //  Language (2 bytes): ISO-639-2/T code (e.g., 0x756E64 = "und" for undefined).
    //
    //  Predefined (2 bytes): Reserved, typically 0x0000.
    //
    int time_scale;
    unsigned int durationv0;
    unsigned long durationv1;
    unsigned short language;
    unsigned short predefined;

    file.read( (char *)&time_scale, 4 );
    if( ver.version == 0 )
    {
        file.read( (char *)&durationv0, 4 );
    } else {
        file.read( (char *)&durationv1, 8 );
    }
    file.read( (char *)&language, 2 );
    file.read( (char *)&predefined, 2 );

    // display the info
    std::cout.imbue(std::locale(""));

    std::cout << std::endl;
    std::cout << padding << "  timescale <" << swapOrder(time_scale) << " per sec>";
    std::cout << ", duration <\033[1;35m";
    if( ver.version == 0 ) std::cout << (float)(swapOrder(durationv0))/swapOrder(time_scale) << " secs\033[0m>";
    else std::cout << durationv1;
    std::cout.imbue(std::locale::classic());
    std::cout << " language <" << decode_lang(swapShort(language)) << "> " << std::endl;

    m_depth--;
}