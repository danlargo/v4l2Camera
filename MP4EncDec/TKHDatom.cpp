
#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseTKHDatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    // read the version and flags
    //
    //  Bit 0 (Value: 0x000001) – Track Enabled
    //      Meaning: If set (1), the track is enabled and should be processed/played. If unset (0), the track is disabled and typically ignored by players.
    //
    //  Bit 1 (Value: 0x000002) – Track in Movie
    //      Meaning: If set (1), the track is used in the main presentation (the "movie"). If unset (0), the track is not part of the primary playback but might be used elsewhere (e.g., in a preview or external reference).
    //
    //  Bit 2 (Value: 0x000004) – Track in Preview
    //      Meaning: If set (1), the track is used in the movie’s preview (e.g., a thumbnail or short snippet). If unset (0), it’s not part of the preview.

    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << "> ";
    if( ver.flags[2] > 0 )
    {
        std::cout << "(";
        if( ver.flags[2] & 0x01 ) std::cout << "enabled,";
        if( ver.flags[2] & 0x02 ) std::cout << "in movie,";
        if( ver.flags[2] & 0x04 ) std::cout << "in preview";
        std::cout << ")";
    }

    parseTIMEhdr( file, ver.version, " " );

    // now parse the rest of the header
    //
    //  Version (1 byte): Usually 0x00 (version 0) or 0x01 (version 1).
    //
    //  Flags (3 bytes): Bitfield (e.g., 0x000001 = track enabled, 0x000002 = track in movie, 0x000004 = track in preview).
    //
    //  Creation Time (4 or 8 bytes, depending on version): Seconds since 1904-01-01 UTC.
    //
    //  Modification Time (4 or 8 bytes, depending on version ): Same epoch.
    //
    //  Track ID (4 bytes): Unique identifier for this track.
    //
    //  Reserved (4 bytes): 0x00000000.
    //
    //  Duration (4 or 8 bytes): Track length in timescale units (from mvhd or mdhd).
    //
    //  Reserved (8 bytes): Zeros.
    //
    //  Layer (2 bytes): Stacking order (e.g., 0x0000 for main track).
    //
    //  Alternate Group (2 bytes): Grouping for alternate tracks (e.g., 0x0000 if none).
    //
    //  Volume (2 bytes): 8.8 fixed-point (e.g., 0x0100 = 1.0 for audio, 0x0000 for video).
    //
    //  Reserved (2 bytes): Zeros.
    //
    //  Matrix (36 bytes): 3x3 transformation matrix (16.16 fixed-point), usually identity for no transformation.
    //
    //  Width (4 bytes): 16.16 fixed-point, display width in pixels.
    //
    //  Height (4 bytes): 16.16 fixed-point, display height in pixels.
    //
    //  Total: 8 (header) + 84 (payload) = 92 bytes for version 0; 104 bytes for version 1.
    //
    int track_id, reserved1;
    unsigned int durationv0;
    unsigned long durationv1;

    file.read( (char *)&track_id, 4 );
    file.read( (char *)&reserved1, 4 );
    if( ver.version == 0 )
    {
        file.read( (char *)&durationv0, 4 );
    } else {
        file.read( (char *)&durationv1, 8 );
    }

    #pragma pack(push, 1)
    struct tkhd_t
    {
        unsigned char reserved2[8];
        unsigned short layer;
        unsigned short altGroup;
        unsigned char volume[2];
        unsigned short reserved3;
        unsigned int matrix[9];
        unsigned short width[2];
        unsigned short height[2];
    } tkhd;
    #pragma pack(pop)

    file.read( (char *)&tkhd, sizeof(tkhd_t) );

    // dump the data
    std::cout << std::endl;
    std::cout << padding << "  track [\033[1;35m" << swapOrder(track_id) << "\033[0m]";
    std::cout << ", duration <\033[1;35m";
    if( ver.version == 0 ) std::cout << (float)(swapOrder(durationv0))/m_timescale << " secs\033[0m>";
    else std::cout << durationv1 << " secs\033[0m> ";
    std::cout << ", size <\033[1;35m" << swapShort(tkhd.width[0]); // << "." << swapShort(tkhd.width[1]);
    std::cout << "x" << swapShort(tkhd.height[0]) << "\033[0m>"; //<< "." << swapShort(tkhd.height[1]) << " >" << std::endl;
    std::cout << ", layer <" << swapShort(tkhd.layer);
    std::cout << ">, alt group <" << swapShort(tkhd.altGroup);
    std::cout << ", volume <" << (int)tkhd.volume[0] << "." << (int)tkhd.volume[1] << ">" << std::endl;

    m_depth--;
}