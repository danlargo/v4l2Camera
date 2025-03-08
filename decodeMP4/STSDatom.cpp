#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseSTSDatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">,";

    len -= 4;

    // now parse the rest of the header
    //
    //  Version (1 byte): Typically 0x00 (version 0 is standard; other versions are rare).
    //
    //  Flags (3 bytes): Usually 0x000000 (reserved, no defined flags).
    //
    //  Payload
    //      Entry Count (4 bytes): A 32-bit unsigned integer specifying the number of sample entries that follow. Usually 0x00000001 (one entry per track), as multiple codecs per track are uncommon.
    //
    //      Sample Entries (variable length): One or more entries, each describing a codec configuration. The format depends on the media type (e.g., video, audio, metadata).
    //
    //      Sample Entry Format
    //          Each sample entry starts with a common header, followed by codec-specific data:
    //              Size (4 bytes): Size of this sample entry (including codec-specific extensions).
    //
    //              Data Format (4 bytes): FourCC identifying the codec (e.g., avc1 for H.264, mp4a for AAC, gpmd for GPMF).
    //
    //              Reserved (6 bytes): 0x000000000000 (padding).
    //
    //              Data Reference Index (2 bytes): Index into the dref table (usually 0x0001 for self-contained files).
    //
    //              Codec-Specific Data: Varies by Data Format (e.g., avcC for H.264).
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

        unsigned int reserved;
        file.read( (char *)&reserved, 6 );
        len -= 6;

        unsigned short ref;
        file.read( (char *)&ref, 2 );
        len -=2;

        std::cout << " ref <" << swapShort(ref) << ">, ";

        int real_size = swapOrder(size) - 16;

        if( real_size > 0 )
        {
            // check fi we know the codec
            if( toUpper(type) == "AVC1") 
            {
                std::cout << "\033[1;35mH.264 codec\033[0m" << std::endl;
                parseH264codec( file, real_size );

            } 
            
            else parseUNKNatom( file, real_size );
        }

        len -= real_size;
    }

    m_depth--;

}

void parseAACcodec( std::ifstream &file, unsigned int size )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    // Data format for AAC codec - STSD atom
    //
    //  Data Format (4 bytes): 0x6D703461 (ASCII mp4a, MPEG-4 Audio).
    //
    //  Reserved (4 bytes): 0x0000.
    //
    //  Reserved (4 bytes): 0x0000.
    //
    //  Channel Count (2 bytes): Number of audio channels (e.g., 0x0002 for stereo).
    //
    //  Sample Size (2 bytes): Bits per sample (e.g., 0x0010 for 16-bit).
    //
    //  Predefined (2 bytes): 0x0000 (reserved).
    //
    //  Reserved (2 bytes): 0x0000.
    //
    //  Sample Rate (4 bytes): Sampling frequency in 16.16 fixed-point format (e.g., 0xAC440000 for 44100 Hz).


    //
    unsigned char reserved[8];
    file.read( (char *)reserved, 8 );
    size -= 8;

    unsigned short channels;
    file.read( (char *)&channels, 2 );
    size -= 2;
    channels = swapShort(channels);

    unsigned short sample_size;
    file.read( (char *)&sample_size, 2 );
    size -= 2;
    sample_size = swapShort(sample_size);

    unsigned int reserved2;
    file.read( (char *)&reserved2, 4 );
    size -= 4;

    unsigned short sample_rate[2];
    file.read( (char *)&sample_rate, 4 );
    size -= 4;
    sample_rate[0] = swapShort(sample_rate[0]);

    // display the data
    std::cout << padding << "  channels <" << channels << ">, sample size <" << sample_size << " bits>, sample rate <" << sample_rate[0] << " Hz>" << std::endl;

    // now parse the codec specific data
    //
    //  Codec Specific Data (variable length): AAC-specific data, typically an Audio
    //
    while( size > 0 )
    {
        // get size and tags for the codec specific data
        unsigned int c_size;
        char type[5]; type[4] = 0;
        file.read( (char *)&c_size, 4 );
        file.read( (char *)type, 4 );
        size -= 8;

        c_size = swapOrder(c_size)-8;

        std::cout << padding << "[\033[1;32m" << toUpper(type) << "\033[0m] " << c_size << " bytes,";
        if( toUpper(type) == "ESD" ) parseESDSconfig( file, c_size );   // ESDS

        else parseUNKNatom( file, c_size );

        size -= c_size;
    }
}

void parseH264codec( std::ifstream &file, unsigned int size )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    // Data format for AVC1 / H264 codec  - STSD atom
    //    
    //  Predefined (2 bytes): 0x0000.
    //  Reserved (2 bytes): 0x0000.
    //  Predefined (12 bytes): 0x00000000 00000000 00000000.
    //
    //  Width (2 bytes): e.g., 0x0400 (1024 pixels).
    //  Height (2 bytes): e.g., 0x0300 (768 pixels).
    //
    //  Horizontal Resolution (4 bytes): 0x00480000 (72 dpi, 16.16 fixed-point).
    //  Vertical Resolution (4 bytes): 0x00480000.
    //
    //  Reserved (4 bytes): 0x00000000.
    //
    //  Frame Count (2 bytes): 0x0001 (1 frame per sample).
    //  Compressor Name (32 bytes): 0x00... (empty, padded).
    //  Depth (2 bytes): 0x0018 (24-bit color).
    //
    //  Predefined (2 bytes): 0xFFFF.
    //
    // grab the reserved bytes
    //
    unsigned char reserved[16];
    file.read( (char *)reserved, 16 );
    size -= 16;
    
    unsigned short width;
    file.read( (char *)&width, 2 );
    size -= 2;
    width = swapShort(width);

    unsigned short height;
    file.read( (char *)&height, 2 );
    size -=2;
    height = swapShort(height);

    unsigned short hres[2];
    file.read( (char *)&hres, 4 );
    size -= 4;
    hres[0] = swapShort(hres[0]);

    unsigned short vres[2];
    file.read( (char *)&vres, 4 );
    size -= 4;
    vres[0] = swapShort(vres[0]);

    unsigned int reserved2;
    file.read( (char *)&reserved2, 4 );
    size -= 4;

    std::cout   << padding << "  size <\033[1;35m" << width << "x" << height << "\033[0m>,"
                << " res <\033[1;35mh: " << hres[0] << " dpi, v: " << vres[0] << " dpi>\033[0m" << std::endl;

    unsigned short frame_count;
    file.read( (char *)&frame_count, 2 );
    size -= 2;
    frame_count = swapShort(frame_count);

    char compressor[33];    compressor[32] = 0;
    file.read( (char *)compressor, 32 );
    size -= 32;

    unsigned short depth;
    file.read( (char *)&depth, 2 );
    size -= 2;
    depth = swapShort(depth);

    unsigned short reserved3;
    file.read( (char *)&reserved3, 2 );
    size -= 2;

    // display the data
    std::cout << padding << "  frame count <" << frame_count << ">, color depth <" << depth << " bits>, compressor name [\033[1;35m" << compressor << "\033[0m]" << std::endl;


    while( size > 0 )
    {
        unsigned int c_size;
        file.read( (char *)&c_size, 4 );
        size -= 4;

        if( c_size == 0 || size == 0 ) break;
        
        char type[5]; type[4] = 0;
        file.read( (char *)type, 4 );
        size -= 4;

        c_size = swapOrder(c_size)-8;

        std::cout << padding << "[\033[1;32m" << toUpper(type) << "\033[0m] ";

        // check the configuration block type
        if( toUpper(type) == "COLR" ) parseCOLRconfig( file, c_size );
        else if( toUpper(type) == "AVCC" ) parseAVCCconfig( file, c_size );

        else parseUNKNatom( file, c_size );

        size -= c_size;
    }

    m_depth--;

}


void parseESDSconfig( std::ifstream &file, unsigned int size )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    //  Version (1 byte): 0x00.
    //  Flags (3 bytes): 0x000000.
    //
    //  Descriptor Data (variable):
    //      ES_Descriptor (tag 0x03):
    //          Tag (1 byte): 0x03.
    //          Length (variable, often 1 byte): e.g., 0x1E (30 bytes).
    //          ES_ID (2 bytes): 0x0000 (unused in MP4)
    //          Flags (1 byte): 0x00.
    //
    //      DecoderConfigDescriptor (tag 0x04):
    //          Tag (1): 0x04.
    //          Length: e.g., 0x15 (21 bytes).
    //          Object Type (1): 0x40 (MPEG-4 AAC).
    //          Stream Type (1): 0x15 (Audio Stream, 5 << 2 | 1).
    //          Buffer Size (3): e.g., 0x000000 (default).
    //          Max Bitrate (4): e.g., 0x00017B48 (96000 bps).
    //          Avg Bitrate (4): e.g., 0x00017B48.
    //
    //      DecoderSpecificInfo (tag 0x05):
    //          Tag (1): 0x05.
    //          Length: e.g., 0x02 (2 bytes).
    //          AudioSpecificConfig (2+): AAC config (e.g., 0x1210 for AAC-LC, 44.1 kHz, stereo).
    //          SLConfigDescriptor (tag 0x06):
    //          Tag (1): 0x06.
    //          Length: 0x01.
    //
    //  Predefined (1): 0x02 (MP4 reserved).

    unsigned char ver;
    file.read( (char *)&ver, 1 );
    size -= 1;

    unsigned char flags[3];
    file.read( (char *)flags, 3 );
    size -= 3;

    std::cout << " ver <0x" << std::hex << (int)ver << std::dec << ">, flags <0x" << std::hex << (int)flags[0] << (int)flags[1] << (int)flags[2] << std::dec << ">" << std::endl;

    // get the descriptors
    while( size > 0 )
    {
        unsigned char tag;
        file.read( (char *)&tag, 1 );
        size -= 1;

        unsigned char len[3];
        file.read( (char *)&len, 3 );
        size -= 3;

        int real_len = 0;

        if( len[0] != 0x80 ) real_len += len[0];
        if( len[1] != 0x80 ) real_len += len[1] << 8;
        if( len[2] != 0x80 ) real_len += len[2] << 16;

        std::cout << padding << "  tag <0x" << std::hex << (int)tag << std::dec << ">, len <" << real_len << " bytes>";

        if( real_len > 0 )
        {
            if( tag == 0x03 )
            {
                unsigned short es_id;
                file.read( (char *)&es_id, 2 );
                size -= 2;
                es_id = swapShort(es_id);

                unsigned char flags;
                file.read( (char *)&flags, 1 );
                size -= 1;

                std::cout << ", es_id <" << es_id << ">, flags <" << (int)flags << ">" << std::endl;

            } else if( tag == 0x04 )
            {
                unsigned char obj_type;
                file.read( (char *)&obj_type, 1 );
                size -= 1;

                unsigned char stream_type;
                file.read( (char *)&stream_type, 1 );
                size -= 1;

                unsigned int buffer_size;
                file.read( (char *)&buffer_size, 3 );
                size -= 3;
                buffer_size = swapOrder(buffer_size);

                unsigned int max_bitrate;
                file.read( (char *)&max_bitrate, 4 );
                size -= 4;
                max_bitrate = swapOrder(max_bitrate);

                unsigned int avg_bitrate;
                file.read( (char *)&avg_bitrate, 4 );
                size -= 4;
                avg_bitrate = swapOrder(avg_bitrate);

                std::cout << ", obj_type <0x" << std::hex << (int)obj_type << std::dec << ">, stream_type <0x" << std::hex << (int)stream_type << std::dec << ">, buffer_size <" << buffer_size << ">, max_bitrate <" << max_bitrate << ">, avg_bitrate <" << avg_bitrate << ">" << std::endl;

            } else 
            {
                unsigned char * data = new unsigned char[real_len];
                file.read( (char *)data, real_len );
                size -= real_len;

                std::cout << "  data <";
                for( int i = 0; i < real_len; i++ ) std::cout << std::hex << (int)data[i] << std::dec << " ";
                std::cout << ">" << std::endl;
            }
        }
    }

}


void parseCOLRconfig( std::ifstream &file, unsigned int size )
{
    int disp_len = size;
    // colr (Color Information Box):Payload
    //
    //  The payload depends on the Colour Type, a 4-byte FourCC that follows the header. Three types are defined:
    //
    //  nclx (Non-Constant Luminance, Extended): Most common, used for modern video (e.g., H.264).
    //
    //  nclc (Non-Constant Luminance, Compact): Older QuickTime format.
    //
    //  prof (ICC Profile): Embeds a full ICC color profile (variable length).
    //
    //      Since nclx is the standard for H.264 video today, I’ll assume this type unless your hex dump suggests otherwise.
    //
    //      nclx Payload
    //          Colour Type (4 bytes): 0x6E636C78 (nclx).
    //          Colour Primaries (2 bytes): Defines the chromaticity coordinates (e.g., BT.709).
    //          Transfer Characteristics (2 bytes): Gamma/transfer function (e.g., sRGB, BT.709).
    //          Matrix Coefficients (2 bytes): Color matrix for RGB-to-YUV conversion (e.g., BT.709).
    //          Full Range Flag (1 byte): Bitfield:
    //          Bit 7 (MSB): 1 = full range (0-255), 0 = video range (16-235 for Y).
    //          Bits 6-0: Reserved (0).
    //
    //          Total Size: 19 bytes (8 header + 11 payload).
    //
    //      nclc Payload 
    //          Colour Type (4 bytes): 0x6E636C63 (nclc).
    //          Colour Primaries (1 byte): 0x01 (BT.709).
    //          Transfer Characteristics (1 byte): 0x01 (BT.709).
    //          Matrix Coefficients (1 byte): 0x01 (BT.709).
    //
    //          Total Size: 18 bytes (8 header + 10 payload).

    char type[5]; type[4] = 0;
    file.read( (char *)type, 4 );
    size -= 4;

    unsigned short primaries;
    file.read( (char *)&primaries, 2 );
    size -= 2;
    primaries = swapShort(primaries);

    unsigned short transfer;
    file.read( (char *)&transfer, 2 );
    size -= 2;
    transfer = swapShort(transfer);

    unsigned short matrix;
    file.read( (char *)&matrix, 2 );
    size -= 2;
    matrix = swapShort(matrix);

    if( toUpper(type) == "NCLX" )
    {
        unsigned char range;
        file.read( (char *)&range, 1 );
        size -= 1;

        // display the data
        std::cout << disp_len << " bytes, type <\033[1;32m" << toUpper(type) << "\033[0m>, primaries <0x" << std::hex << primaries << ">, transfer <0x" << transfer << ">, matrix <0x" << matrix << std::dec << ">, range <" << (int)range << ">" << std::endl;

    } else {
        // display the data, without the range field
        std::cout << disp_len << " bytes, type <\033[1;32m" << toUpper(type) << "\033[0m>, primaries <0x" << std::hex << primaries << ">, transfer <0x" << transfer << ">, matrix <0x" << matrix << ">" << std::dec << std::endl;
    }
 
}

void parseAVCCconfig( std::ifstream &file, unsigned int size )
{
    int disp_len = size;

    m_depth++;
    std::string padding = calcPadding(m_depth);

    //  avcC (AVC Configuration Box):
    //
    //      Size: 0x0000001C (28 bytes, example with short SPS/PPS).
    //      Type: 0x61766343 (avcC).
    //      Configuration Version: 0x01.
    //      AVC Profile: e.g., 0x42 (Baseline).
    //      Profile Compatibility: 0x00.
    //      AVC Level: e.g., 0x1E (Level 3.0).
    //      Reserved + Length Size Minus One: 0xFF (6 bits reserved, 2 bits = 3 → 4-byte length).
    //      Num SPS: 0xE1 (1 SPS, high bit set).
    //      SPS Length: 0x000A (10 bytes).
    //      SPS Data: e.g., 0x67 42 C0 1E ....
    //      Num PPS: 0x01 (1 PPS).
    //      PPS Length: 0x0004 (4 bytes).
    //      PPS Data: e.g., 0x68 CE 06 E2.

    unsigned char ver;
    file.read( (char *)&ver, 1 );
    size -= 1;

    unsigned char profile;
    file.read( (char *)&profile, 1 );
    size -= 1;

    std::string profile_str = "custom";
    switch( profile )
    {
        case 0x42: profile_str = "baseline"; break;
        case 0x4D: profile_str = "main"; break;
        case 0x58: profile_str = "extended"; break;
        case 0x64: profile_str = "high"; break;
        case 0x66: profile_str = "high 10"; break;
        case 0x6E: profile_str = "high 4:2:2"; break;
        case 0x7A: profile_str = "high 4:4:4"; break;
    }

    unsigned char compatibility;
    file.read( (char *)&compatibility, 1 );
    size -= 1;

    unsigned char level;
    file.read( (char *)&level, 1 );
    size -= 1;

    unsigned char reserved4;
    file.read( (char *)&reserved4, 1 );
    size -= 1;

    unsigned char num_sps;
    file.read( (char *)&num_sps, 1 );
    size -= 1;
    num_sps = num_sps & 0x1F;

    unsigned short sps_len;
    file.read( (char *)&sps_len, 2 );
    size -= 2;
    sps_len = swapShort(sps_len);

    unsigned char * sps = new unsigned char[sps_len];
    file.read( (char *)sps, sps_len );
    size -= sps_len;

    unsigned char num_pps;
    file.read( (char *)&num_pps, 1 );
    size -= 1;

    unsigned short pps_len;
    file.read( (char *)&pps_len, 2 );
    size -= 2;
    pps_len = swapShort(pps_len);

    unsigned char * pps;
    if( pps_len > 0 )
    {
        pps = new unsigned char[pps_len];
        file.read( (char *)pps, pps_len );
        size -= pps_len;
    }

    std::cout << std::dec << disp_len << " bytes, ver <0x" << std::hex << (int)ver << std::dec << ">, profile <0x" << std::hex << (int)profile << std::dec << ", " << profile_str << ">, compat <" << (int)compatibility << ">, level <" << (int)level << ">" << std::endl;
    std::cout << padding << "  num_sps <" << (int)num_sps << ">, sps_len <" << sps_len << ">, data <";
    for( int i = 0; i < sps_len; i++ ) std::cout << std::hex << (int)sps[i] << std::dec << " ";
    std::cout << ">" << std::endl;
    
    std::cout << padding << "  num_pps <" << (int)num_pps << ">, pps_len <" << pps_len << ">, data <" ;
    for( int i = 0; i < pps_len; i++ ) std::cout << std::hex << (int)pps[i] << std::dec << " ";
    std::cout << ">" << std::endl;

    delete [] sps;
    if( pps_len > 0 ) delete [] pps;

    // check for spare data
    if( size > 0 )
    {
        unsigned char * fld = new unsigned char[size];
        file.read( (char *)fld, size);
        std::cout << padding << "  remaining data < ";
        for( int i = 0; i < size; i++ ) std::cout << std::hex << (int)fld[i] << std::dec << " ";
        std::cout << ">" << std::endl;
        delete [] fld;
    }

    m_depth--;

}
