#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseMVHDatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    // read the version and flags
    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">  ";

    //     Creation Time:
    //          Version 0: 4 bytes (32-bit unsigned integer).
    //          Version 1: 8 bytes (64-bit unsigned integer).
    //              Represents seconds since midnight, January 1, 1904 (UTC), the epoch used by QuickTime and MP4.
    //
    //      Modification Time:
    //          Version 0: 4 bytes.
    //          Version 1: 8 bytes.
    //              Same epoch as creation time.
    //
    //      Timescale (4 bytes): A 32-bit unsigned integer defining the number of time units per second for the movie. For example, a timescale of 1000 means durations are measured in milliseconds.
    //
    //      Duration:
    //          Version 0: 4 bytes (32-bit unsigned integer).
    //          Version 1: 8 bytes (64-bit unsigned integer).
    //              The total length of the movie in timescale units. Divide this by the timescale to get the duration in seconds.

    //      Preferred Rate (4 bytes): A 32-bit fixed-point number (16.16 format, where the first 16 bits are the integer part and the last 16 are the fractional part). Typically set to 0x00010000 (1.0), indicating normal playback speed.
    //
    //      Preferred Volume (2 bytes): A 16-bit fixed-point number (8.8 format). Usually 0x0100 (1.0), representing full volume.
    //
    //      Reserved (10 bytes): Set to all zeros. This padding aligns the following fields.
    //
    //      Matrix (36 bytes): A 3x3 transformation matrix (9 elements, each 4 bytes in 16.16 fixed-point format). Defines the spatial transformation of the video (e.g., rotation, scaling). The default identity matrix is:
    //          [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0] in fixed-point form: 0x00010000, 0x00000000, 0x00000000, 0x00000000, 0x00010000, 0x00000000, 0x00000000, 0x00000000, 0x40000000.
    //
    //      Predefined (24 bytes): Six 32-bit fields (total 24 bytes), reserved and set to zero.
    //
    //      Next Track ID (4 bytes): A 32-bit unsigned integer specifying the next available track ID for the movie. Tracks in the MP4 file use this to assign unique identifiers.
    //
    //      Structure Size
    //      Version 0: 8 (header) + 100 (payload) = 108 bytes.
    //      Version 1: 8 (header) + 112 (payload, due to larger time and duration fields) = 120 bytes.

    parseTIMEhdr( file, ver.version, "" );

    #pragma pack(push, 1)
    struct mvhd_t
    {
        unsigned int timescale;
        unsigned int duration;
        unsigned short rate[2];
        unsigned char volume[2];
        unsigned char reserved[10];
        unsigned int matrix[9];
        unsigned int predefined[6];
        unsigned int nextTrackID;
    } mvhd;
    #pragma pack(pop)

    // now read the rest
    file.read( (char *)&mvhd, sizeof(mvhd_t) );

    std::cout.imbue(std::locale(""));

    std::cout << std::endl;
    std::cout << padding << "  duration <\033[1;35m" << (float)(swapOrder(mvhd.duration))/swapOrder(mvhd.timescale) << " secs\033[0m>";
    std::cout << ", timescale < " << swapOrder(mvhd.timescale);
    std::cout << " >, playback rate < " << swapShort(mvhd.rate[0]) << "." << swapShort(mvhd.rate[1]);
    std::cout << " >, volume < " << (int)mvhd.volume[0] << "." << (int)mvhd.volume[1] << " > " << std::endl;
    std::cout << padding << "  next track [\033[1;35m" << swapOrder(mvhd.nextTrackID) << "\033[0m]" << std::endl;

    // save the timescale for later
    m_timescale = swapOrder(mvhd.timescale);

    m_depth--;
}

