#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseSTTSatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">,";

    len -= 4;

    // now parse the rest of the header
    //
    //  Version (1 byte): Typically 0x00 (version 0 is standard; version 1 uses 64-bit durations, rare).
    //
    //  Flags (3 bytes): Usually 0x000000 (reserved, no defined flags).
    //
    //  Payload
    //      Entry Count (4 bytes): A 32-bit unsigned integer specifying the number of entries in the table that follows. Each entry describes a run of consecutive samples with the same duration.
    //
    //      Sample Table (variable length): An array of entries, where each entry consists of:
    //          Sample Count (4 bytes): Number of consecutive samples with the same duration.
    //          Sample Delta (4 bytes): Duration of each sample in timescale units (e.g., ticks).
    //
    //  Total Size: 16 bytes (header) + 8 bytes per entry × Entry Count.
    //
    unsigned int count;
    file.read( (char *)&count, 4 );
    len -= 4;

    std::cout << " count <" << swapOrder(count) << ">";

    unsigned int sample_count;
    file.read( (char *)&sample_count, 4 );
    len -= 4;
    sample_count = swapOrder(sample_count);


    unsigned int sample_delta;
    file.read( (char *)&sample_delta, 4 );
    len -= 4;
    sample_delta = swapOrder(sample_delta);

    std::cout << "  sample count <" << sample_count << ">, sample delta <" << sample_delta << " ticks>" << std::endl;

    // read the remaining sample data
    if( len > 0 )
    {
        unsigned char * fld = new unsigned char[len];
        file.read( (char *)fld, len);
        delete [] fld;
    }

    m_depth--;
}

