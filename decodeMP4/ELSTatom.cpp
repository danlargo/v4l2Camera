#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseELSTatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">, ";

    // now parse the rest of the header
    //
    //  Entry Count (4 bytes):
    //      A 32-bit unsigned integer specifying the number of edit entries that follow.
    //
    //  Edit List Entries (variable length, Entry Count × entry size):
    //      Each entry describes a segment of the track’s presentation. The size of each entry depends on the Version:
    //          Version 0 (32-bit): 12 bytes per entry.
    //          Version 1 (64-bit): 20 bytes per entry.
    //
    //  Fields per entry:
    //
    //      Segment Duration (4 or 8 bytes):
    //          Duration of the edit in the track’s timescale (from mdhd).
    //              If 0, it’s an empty edit (delay before media starts).
    //              If -1 (0xFFFFFFFF for version 0), it’s a dwell (repeat the first frame).
    //
    //      Media Time (4 or 8 bytes):
    //          Starting time within the media (in timescale units) for this segment.
    //              If -1 (0xFFFFFFFF), it’s an empty edit (no media played).
    //              Otherwise, maps to a sample in the track.
    //
    //      Media Rate (4 bytes, 16.16 fixed-point):
    //          Playback speed as a fixed-point number (e.g., 0x00010000 = 1.0, normal speed).
    //          Rarely deviates from 1.0 in practice.
    //
    unsigned int count;

    #pragma pack(push, 1)
    struct elst_v0_t
    {
        unsigned int duration;
        unsigned int media_time;
        unsigned short media_rate[2];
    } elst_v0;
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct elst_v1_t
    {
        unsigned long duration;
        unsigned long media_time;
        unsigned short media_rate[2];
    } elst_v1;
    #pragma pack(pop)

    file.read( (char *)&count, 4 );
    std::cout << "count <" << swapOrder(count) << ">" << std::endl;
    std::cout.imbue(std::locale(""));

    for( int i = 0; i < swapOrder(count) ; i++ )
    {
        if( ver.version == 0 )
        {
            file.read( (char *)&elst_v0.duration, 4 );
            file.read( (char *)&elst_v0.media_time, 4 );
            file.read( (char *)&elst_v0.media_rate, 4 );
            std::cout << padding << "  [" << (i+1) << "] duration <\033[1;35m" << (float)(swapOrder(elst_v0.duration))/m_timescale << " secs\033[0m>, media time <" << swapOrder(elst_v0.media_time);
            std::cout << ">, rate <" << swapShort(elst_v0.media_rate[0]) << "." << swapShort(elst_v0.media_rate[1]) << ">, timescale <" << m_timescale << ">" << std::endl;
        } else {
            file.read( (char *)&elst_v1, sizeof(elst_v1) );
            std::cout << padding << "  [" << (i+1) << "] duration <" << elst_v1.duration/m_timescale << ">, media time <" << elst_v1.media_time;
            std::cout << ">, rate <" << elst_v1.media_rate[0] << "." << elst_v1.media_rate[1] << ">, timescale <" << m_timescale << ">" << std::endl;
        }
    }

    m_depth--;
}