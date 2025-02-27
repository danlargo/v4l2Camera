#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

// Version Info
//
const int s_majorVersion = 0;
const int s_minorVersion = 1;
const int s_revision = 10;

int main( int argc, char** argv )
{
    // set the flags
    bool onlyFrameHeaders = false;
    bool fileNameProvided = false;
    bool printHelp = false;
    bool printVersion = false;

    // indicate the versions we know
    known_mp4[0].name = "isom";     known_mp4[0].ver = 0;
    known_mp4[1].name = "isom";     known_mp4[1].ver = 1;
    known_mp4[2].name = "isom";     known_mp4[2].ver = 0x200;
    num_known_mp4 = 3;

    std::string filename = "";
    char * buffer;
    std::ifstream file;

    // parse the command line
    for( int i = 1; i < argc; i++ )
    {
        std::string cmp = argv[i];

        if( cmp == "-f" ) onlyFrameHeaders = true;
        if( (cmp == "-h") || (cmp == "--help") ) printHelp = true;
        if( cmp == "-v" ) printVersion = true;
        if( cmp == "-i" ) 
        {
            // make sure there is a filename
            if( ((i+1) < argc) && (argv[i+1][0] != '-') )
            {
                filename = argv[++i];
                fileNameProvided = true;
            } else std::cerr << "[\x1b[1;31mwarning\x1b[0m] : invalid filename or filename not provided, defaulting to STDIN" << std::endl;
        }
    }
        
    // check for command line arguments and then bail
    if( (argc == 1) || printHelp)
    {
        std::cout <<    "Usage: " << argv[0] << " " << getVersionString() << std::endl << std::endl <<
                        " [-h|--help]       : this message" << std::endl <<
                        " [-v]              : print version information" << std::endl <<
                        " [-f]              : decode frame headers only, streaming from UVC camera" << std::endl <<
                        "                       ...default is to try to decode the MP4 stream header" << std::endl <<
                        " [-i filename]     : decode MP4 file" << std::endl <<
                        "                       ...default is to decode from std:cin" << std::endl; 
        return 0;
    }

    // do the version thing
    if( printVersion )
    {
        std::cout << argv[0] << " Version : " << getVersionString() << ", (c) copyright 2025, All rights reserved, slapfrog Labs" << std::endl;
        return 0;
    }

    // try to open the file
    if( !fileNameProvided ) filename = "/dev/stdin";

    file.open( filename, std::ios::in | std::ios::binary );
    if( !file.is_open() )
    {
        std::cerr << "[\x1b[1;31mwarning\x1b[0m] : could not open file " << filename << std::endl;
        return 1;
    }

    // do the work
    if( onlyFrameHeaders ) std::cout << "Decoding MP4 frame headers ";
    else std::cout << "Decoding MP4 stream ";
    if( fileNameProvided ) std::cout << "from file " << filename << std::endl;
    else std::cout << "from STDIN" << std::endl;

    // read the file
    // MP4 file format defined here https://www.loc.gov/preservation/digital/formats/fdd/fdd000155.shtml
    //
    // series of ATOM tags followed by data
    //
    int num_atoms = 0;
    int file_size = 0;
    while( !file.eof() )
    {
        // read the atom and then decide that to do
        struct atom_t atom;
        
        file.read( (char *)&atom, 8 );
        
        // re-build the size
        int real_size = swapOrder(atom.size);
        // print out the atom tag and size
        std::cout << std::setw(2) << (num_atoms+1) << " : Atom [ " << atom.type[0] << atom.type[1] << atom.type[2] << atom.type[3] << " ] "<< real_size << " bytes" << std::endl;
        num_atoms++;
        file_size += real_size;
        //
        // decode the FTYP atom
        //
        if( (atom.type[0] == 'f') && (atom.type[1] == 't') && (atom.type[2] == 'y') && (atom.type[3] == 'p') )
        {
            std::string variant;
            if( !parseFTYPatom( file, real_size, variant ) )
            {
                std::cerr << "[\x1b[1;31mwarning\x1b[0m] : [" << variant << "] no parsing information for this MP4 variant, exiting" << std::endl;
                break;
            }
        //
        // Decode the MOOV atom
        //
        } else if( (atom.type[0] == 'm') && (atom.type[1] == 'o') && (atom.type[2] == 'o') && (atom.type[3] == 'v') )
        {
            parseMOOVatom( file, real_size );

        } else {
            // read the rest of the atoms
            buffer = new char[real_size-8];
            file.read( (char *)buffer, real_size-8 );

            delete [] buffer;
        }

    }

    // close the file
    file.close();

    // display summary
    std::cout << std::endl << "Num atoms : " << num_atoms << std::endl;
    std::cout.imbue(std::locale(""));

    if( file_size < (1024*1024) )
    {
        float file_size_kb = (float)file_size / 1024.0;
        std::cout << "File size : " << std::fixed << std::setprecision(2) << file_size_kb << " KB" << std::endl;    
    } else {
        float file_size_kb = (float)file_size / (1024.0 * 1024.0);
        std::cout << "File size : " << std::fixed << std::setprecision(2) << file_size_kb << " MB" << std::endl;    
    }

    // indicate done
    std::cerr << std::endl << std::endl << argv[0] << " : done" << std::endl;

    return 0;
}


// ATOM Parsers
//
//
// FTYP atom
//
bool parseFTYPatom( std::ifstream &file, int len, std::string& variant )
{
    bool ret = false;
    //
    // FTYP header format
    // Size             : A 32-bit unsigned integer representing the total size of the "ftyp" atom in bytes. 
    // Type             : A 4-byte code, always "ftyp", signifying the atom type. 
    // Major Brand      : A 4-byte code identifying the primary file format (usually "isom"). 
    // Minor Version    : A 32-bit unsigned integer representing the version number of the file format. 
    // Compatible Brands (Optional) : A list of additional 4-byte codes indicating compatible file formats.
    //
    #pragma pack(push, 1)
    struct ftyp_t
    {
        unsigned char majorBrand[4];
        unsigned int minorVersion;
    };
    #pragma pack(pop)

    struct ftyp_t ftyp;
    file.read( (char *)&ftyp, 8 );
    // display
    std::string major_brand = "";
    major_brand += ftyp.majorBrand[0];
    major_brand += ftyp.majorBrand[1];
    major_brand += ftyp.majorBrand[2];
    major_brand += ftyp.majorBrand[3];
    std::cout << "   Major Brand  : " << major_brand << " v" << std::hex << swapOrder(ftyp.minorVersion) << std::dec << std::endl;

    // read the compatible brands, if they are appended
    if( len > 16 )
    {
        char * buffer = new char[len-16];
        file.read( buffer, len-16 );

        // display the compatible brands
        std::cout << "   Compatible Brands : ";
        for( int i = 0; i < (len-16); i+=4 ) std::cout << buffer[i] << buffer[i+1] << buffer[i+2] << buffer[i+3] << " ";
        std::cout << std::endl;

        delete [] buffer;
    }

    // add a blank line
    std::cout << std::endl;

    // check if we know this variant
    ret = checkKnownVariant( major_brand, swapOrder(ftyp.minorVersion) );
    // add the version to the returned string
    major_brand += " v";
    major_brand += std::to_string(swapOrder(ftyp.minorVersion));

    variant = major_brand;

    return ret;
}

bool checkKnownVariant( std::string name, int ver )
{
    bool ret = false;

    for( int i = 0; i < num_known_mp4; i++ )
    {
        if( (known_mp4[i].name == name) && (known_mp4[i].ver == ver) )
        {
            ret = true;
            break;
        }
    }

    return ret;
}


// MOOV atom
//
void parseMOOVatom( std::ifstream &file, int len  )
{
    // read the sub-atoms in the MOOV atom
    char * buffer;
    int remaining = len - 8;
    while( remaining > 0 )
    {
        struct atom_t moov_atom;
        file.read( (char *)&moov_atom, 8 );

        // display it
        int real_moov_size = swapOrder(moov_atom.size);
        std::cout << "   MOOV Atom [ " << moov_atom.type[0] << moov_atom.type[1] << moov_atom.type[2] << moov_atom.type[3] << " ] "<< real_moov_size << " bytes" << std::endl;
        
        remaining -= 8;
        
        if( (moov_atom.type[0] == 'm') && (moov_atom.type[1] == 'v') && (moov_atom.type[2] == 'h') && (moov_atom.type[3] == 'd') )
        {
            parseMVHDatom( file, real_moov_size-8 );

        } else if( (moov_atom.type[0] == 't') && (moov_atom.type[1] == 'r') && (moov_atom.type[2] == 'a') && (moov_atom.type[3] == 'k') )
        {
            parseTRAKatom( file, real_moov_size-8 );

        } else if( (moov_atom.type[0] == 'i') && (moov_atom.type[1] == 'o') && (moov_atom.type[2] == 'd') && (moov_atom.type[3] == 's') )
        {
            parseIODSatom( file, real_moov_size-8 );

        } else {
            // discard the atom
            buffer = new char[real_moov_size-8];
            file.read( buffer, real_moov_size-8 );
            delete [] buffer;
        }
        remaining -= real_moov_size-8;

    }

    // add a blank line
    std::cout << std::endl;
}


void parseMVHDatom( std::ifstream &file, int len )
{
    // read the version and flags
    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "         Version   : " << (int)ver.version << ", Flags : " << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << std::endl;

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

    std::cout << "         Timescale : " << swapOrder(mvhd.timescale) << std::endl;
    std::cout << "         Duration  : " << (float)(swapOrder(mvhd.duration))/swapOrder(mvhd.timescale) << " secs" << std::endl;
    std::cout << "         Rate      : " << swapShort(mvhd.rate[0]) << "." << swapShort(mvhd.rate[1]) << std::endl;
    std::cout << "         Volume    : " << (int)mvhd.volume[0] << "." << (int)mvhd.volume[1] << std::endl;
    std::cout << "         Next Track: " << swapOrder(mvhd.nextTrackID) << std::endl;

    // save the timescale for later
    m_timescale = swapOrder(mvhd.timescale);

    std::cout << std::endl;
}

// TRAK atom
//
void parseTRAKatom( std::ifstream &file, int len )
{
    // read the sub-atoms in the TRAK atom
    char * buffer;
    int remaining = len - 8;
    while( remaining > 0 )
    {
        struct atom_t trak_atom;
        file.read( (char *)&trak_atom, 8 );
        // display it
        int real_trak_size = swapOrder(trak_atom.size);
        std::cout << "      TRAK Atom [ " << trak_atom.type[0] << trak_atom.type[1] << trak_atom.type[2] << trak_atom.type[3] << " ] "<< real_trak_size << " bytes" << std::endl;
        
        remaining -= 8;

        // check if this is a trak header (tkhd)
        if( (trak_atom.type[0] == 't') && (trak_atom.type[1] == 'k') && (trak_atom.type[2] == 'h') && (trak_atom.type[3] == 'd') )
        {
            // read the tkhd atom
            parseTKHDatom( file, real_trak_size-8 );

        }
        // check if this is a trak header (tkhd)
        else if( (trak_atom.type[0] == 'm') && (trak_atom.type[1] == 'd') && (trak_atom.type[2] == 'i') && (trak_atom.type[3] == 'a') )
        {
            // read the tkhd atom
            parseMDIAatom( file, real_trak_size-8 );

        } else {

            // discard the atom
            buffer = new char[real_trak_size-8];
            file.read( buffer, real_trak_size-8 );
            delete [] buffer;
        }
        remaining -= real_trak_size-8;
    }

    // add a blank line
    std::cout << std::endl;
}


void parseTKHDatom( std::ifstream &file, int len )
{
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
    std::cout << "            Version   : " << (int)ver.version << ", Flags : " << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec;
    if( ver.flags[2] > 0 )
    {
        std::cout << "( ";
        if( ver.flags[2] & 0x01 ) std::cout << "Enabled,";
        if( ver.flags[2] & 0x02 ) std::cout << "In Movie,";
        if( ver.flags[2] & 0x04 ) std::cout << "In Preview";
        std::cout << " )";
    }
    std::cout << std::endl;

    parseTIMEhdr( file, ver.version, "   " );

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
    std::cout << "            Track ID  : " << swapOrder(track_id) << std::endl;
    std::cout << "            Duration  : ";
    if( ver.version == 0 ) std::cout << (float)(swapOrder(durationv0))/m_timescale << " secs" << std::endl;
    else std::cout << durationv1 << std::endl;
    std::cout << "            Layer     : " << swapShort(tkhd.layer);
    std::cout << ", Alt Group : " << swapShort(tkhd.altGroup);
    std::cout << ", Volume    : " << (int)tkhd.volume[0] << "." << (int)tkhd.volume[1] << std::endl;
    std::cout << "            Scale     : " << swapShort(tkhd.width[0]) << "." << swapShort(tkhd.width[1]);
    std::cout << " x " << swapShort(tkhd.height[0]) << "." << swapShort(tkhd.height[1]) << std::endl;

    // insert a blank line
    std::cout << std::endl;
}


void parseMDIAatom(  std::ifstream &file, int len )
{
    // this is a container atom
    char * buffer;
    int remaining = len - 8;
    while( remaining > 0 )
    {
        struct atom_t mdai_atom;
        file.read( (char *)&mdai_atom, 8 );
        // display it
        int real_mdia_size = swapOrder(mdai_atom.size);
        std::cout << "         MDIA Atom [ " << mdai_atom.type[0] << mdai_atom.type[1] << mdai_atom.type[2] << mdai_atom.type[3] << " ] "<< real_mdia_size << " bytes" << std::endl;
        
        remaining -= 8;

        // check if this is a trak header (tkhd)
        if( (mdai_atom.type[0] == 'm') && (mdai_atom.type[1] == 'd') && (mdai_atom.type[2] == 'h') && (mdai_atom.type[3] == 'd') )
        {
            parseMDHDatom( file, real_mdia_size-8 );

        } else if( (mdai_atom.type[0] == 'h') && (mdai_atom.type[1] == 'd') && (mdai_atom.type[2] == 'l') && (mdai_atom.type[3] == 'r') )
        {
            parseHDLRatom( file, real_mdia_size-8 );

        } else if( (mdai_atom.type[0] == 'm') && (mdai_atom.type[1] == 'i') && (mdai_atom.type[2] == 'n') && (mdai_atom.type[3] == 'f') )
        {
            parseMINFatom( file, real_mdia_size-8 );

        } else {

            // discard the atom
            buffer = new char[real_mdia_size-8];
            file.read( buffer, real_mdia_size-8 );
            delete [] buffer;
        }
        remaining -= real_mdia_size-8;
    }

    // add a blank line
    std::cout << std::endl;
}


void parseMDHDatom( std::ifstream &file, int len )
{
    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "            Version   : " << (int)ver.version << ", Flags : " << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << std::endl;

    parseTIMEhdr( file, ver.version, "   " );

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

    std::cout << "            Timescale : " << swapOrder(time_scale) << " per sec" << std::endl;
    std::cout << "            Duration  : ";
    if( ver.version == 0 ) std::cout << (float)(swapOrder(durationv0))/swapOrder(time_scale) << " secs" << std::endl;
    else std::cout << durationv1 << std::endl;
    std::cout.imbue(std::locale::classic());
    std::cout << "            Language  : " << decode_lang(swapShort(language)) << std::endl;

    // add a blank line
    std::cout << std::endl;
}


void parseHDLRatom( std::ifstream &file, int len )
{
    //  Version (1 byte):
    //      Typically 0x00. Indicates the version of the hdlr atom format. No other versions are commonly used.
    //
    //  Flags (3 bytes):
    //      Usually 0x000000. Reserved for future use; no specific flags are defined in the standard.
    //
    //  Predefined (4 bytes): Set to 0x00000000. Historically a QuickTime component type, but in MP4 it’s typically zero and ignored.
    //
    //  Handler Type (4 bytes):
    //      A 4-character code identifying the media type of the track. Common values:
    //          0x76696465 (vide) – Video track.
    //          0x736F756E (soun) – Audio track.
    //          0x73756274 (subt) – Subtitle track.
    //          0x74657874 (text) – Text track.
    //          0x68696E74 (hint) – Hint track (for streaming).
    //          0x6D657461 (meta) – Metadata track.
    //      This is the critical field that tells the player what kind of data to expect.
    //
    //      Reserved (12 bytes):
    //
    //      Name (variable length):
    //          A null-terminated string (UTF-8 or ASCII) providing a human-readable description of the handler (e.g., "VideoHandler", "SoundHandler").
    #pragma pack(push, 1)
    struct hdlr_atom_t
    {
        unsigned char version;
        unsigned char flags[3];
        unsigned int predefined;
        union {
            unsigned int h_int;
            char h_str[4];
        };
        unsigned char reserved[12];
    } hdlr_atom;
    #pragma pack(pop)
    char * name = new char[len - sizeof(hdlr_atom_t)];

    file.read( (char *)&hdlr_atom, sizeof(hdlr_atom_t) );
    file.read( (char *)name, len - sizeof(hdlr_atom_t) );

    // display the data
    std::cout << "            Version      : " << (int)hdlr_atom.version << ", Flags : " << std::hex << (int)hdlr_atom.flags[0] << (int)hdlr_atom.flags[1] << (int)hdlr_atom.flags[2] << std::dec << std::endl;
    std::cout << "            Handler Type : 0x" << std::hex << swapOrder(hdlr_atom.h_int) << std::dec << ", ";
    std::cout <<  hdlr_atom.h_str[0] << hdlr_atom.h_str[1] << hdlr_atom.h_str[2] << hdlr_atom.h_str[3] << std::endl;
    std::cout << "            Name         : " << name << std::endl;

    // add a blank line
    std::cout << std::endl;
}

void parseMINFatom( std::ifstream &file, int len )
{
    // this is another container atom
    char * buffer;
    int remaining = len - 8;
    while( remaining > 0 )
    {
        struct atom_t mdai_atom;
        file.read( (char *)&mdai_atom, 8 );
        // display it
        int real_mdia_size = swapOrder(mdai_atom.size);
        std::cout << "            MINF Atom [ " << mdai_atom.type[0] << mdai_atom.type[1] << mdai_atom.type[2] << mdai_atom.type[3] << " ] "<< real_mdia_size << " bytes" << std::endl;
        
        remaining -= 8;

        // check if this is a trak header (tkhd)
        if( (mdai_atom.type[0] == 'm') && (mdai_atom.type[1] == 'h') && (mdai_atom.type[2] == 'd') && (mdai_atom.type[3] == 'h') )
        {

        } else {

            // discard the atom
            buffer = new char[real_mdia_size-8];
            file.read( buffer, real_mdia_size-8 );
            delete [] buffer;
        }
        remaining -= real_mdia_size-8;
    }

    // add a blank line
    std::cout << std::endl;
}


void parseIODSatom( std::ifstream &file, int len )
{
    // get the tag and flags, using the version structure
    //
    //  Version (1 byte): Typically 0x00, indicating the version of the IOD format.
    //
    //  Flags (3 bytes): Usually 0x000000, reserved for future use or extensions. No specific flags are defined in the standard.
    //
    struct version_t ver;
    file.read( (char *)&ver , 4 );
    std::cout << "         Version   : " << (int)ver.version << ", Flags : " << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << std::endl;

    len -= 4;

    // parse the rest
    //
    //  Tag (1 byte): 
    //      Value: 0x02 (ObjectDescriptor tag), indicating this is an Initial Object Descriptor. 
    //          In rare cases, 0x01 (InitialObjectDescriptor tag) might appear, but 0x02 is standard in MP4.
    //
    //  Length (1–4 bytes): 
    //      Variable-length integer encoded per MPEG-4 rules:
    //          If the length < 128, it’s 1 byte.
    //          If 128 ≤ length < 16,384, it’s 2 bytes with the first bit set to 1 (e.g., 0x81 0xXX).
    //          The length specifies the size of the remaining IOD data (excluding tag and length bytes).   
    //          For simple MP4s, this is often 0x07 (7 bytes of data follows).
    //
    //  ObjectDescriptorID (10 bits): 
    //      A 10-bit identifier (0–1023) for the object descriptor, usually 0x01 or a low number. Packed into 2 bytes with other fields.
    //
    //  URL Flag (1 bit): 
    //      0 = No URL (inline descriptor data follows); 1 = URL points to external descriptor (rare in MP4).
    //          Typically 0.
    //
    //  Include Inline Profile Level Flag (1 bit): 
    //      0 = No inline profiles; 1 = Profiles included. Usually 1.
    //
    //  Reserved (4 bits): 
    //  Set to 0xF (all 1s) per the spec.
    //
    //  Profile/Level Indications (5 bytes):
    //      OD Profile/Level (1 byte): Object Descriptor profile (e.g., 0xFF = no profile, common for simple files).
    //      Scene Profile/Level (1 byte): BIFS or scene description profile (e.g., 0xFF = none).
    //      Audio Profile/Level (1 byte): Audio profile (e.g., 0xFF = none, or a value like 0x02 for AAC LC).
    //      Video Profile/Level (1 byte): Video profile (e.g., 0xFF = none, or 0x21 for H.264 Baseline).
    //      Graphics Profile/Level (1 byte): Graphics profile (e.g., 0xFF = none).
    //
    //          These are often all 0xFF in basic MP4s without MPEG-4 Systems complexity.
    //
    //  Optional Sub-Descriptors (variable):
    //      Additional descriptors (e.g., ES_Descriptor for elementary streams) may follow, each with its own tag and length.
    //      In simple MP4s, these are usually absent.
    //
    unsigned char tag;
    unsigned char desc_len;
    unsigned char alt_len;
    unsigned short desc_id;
    unsigned char profile[5];
    char * extra;

    while( len > 0 )
    {
        // get the tag and length
        file.read( (char *)&tag, 1 );
        file.read( (char *)&desc_len, 1 );
        unsigned int real_len = (int)len;
        len -= 2;
        if( len > 127 )
        {
            file.read( (char *)&alt_len, 1);
            len -= 1;
            real_len = alt_len + 128;
        }
        // get the descriptor and details
        file.read( (char *)&desc_id, 2 );
        file.read( (char *)profile, 5 );
        len -= 7;

        std::cout << "         Desc Tag  : 0x" << std::hex <<  (int)tag << std::dec << ", Len : " << real_len << ", Desc ID : 0x" << std::hex << (desc_id & 0x003f) << std::dec << std::endl;
        std::cout << "         URL Flag  : " << ((desc_id & 0x0040) ? "1" : "0") << ", Inline Profile : " << ((desc_id & 0x0080) ? "1" : "0") << std::endl;
        std::cout << "         Profile   : ";
        for( int i = 0; i < 5; i++ ) std::cout << std::hex << "0x" << (int)profile[i] << std::dec << " ";
        std::cout << std::endl;
    }

    // add a blank line
    std::cout << std::endl;
}


void parseTIMEhdr( std::ifstream &file, unsigned char version, std::string padding )
{
    //     Creation Time:
    //          Version 0: 4 bytes (32-bit unsigned integer).
    //          Version 1: 8 bytes (64-bit unsigned integer).
    //              Represents seconds since midnight, January 1, 1904 (UTC), the epoch used by QuickTime and MP4.
    //
    //      Modification Time:
    //          Version 0: 4 bytes.
    //          Version 1: 8 bytes.
    //              Same epoch as creation time.

    if( version == 0 )
    {
        file.read( (char *)&timev0, 8 );

        #define MP4_EPOCH_OFFSET 2082844800ULL  // 66 years, accounting for leap years

        time_t tmp_time = (time_t)(swapOrder(timev0.created) - MP4_EPOCH_OFFSET);
        struct tm *time_info;
        char buffer[80];
        if( swapOrder(timev0.created) == 0 )
        {
            std::cout << padding << "         Created   : -";
        } else {
            time_info = gmtime(&tmp_time);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
            std::cout << padding << "         Created   : " << buffer;
        }

        tmp_time = (time_t)(swapOrder(timev0.modified) - MP4_EPOCH_OFFSET);
        if( swapOrder(timev0.modified) == 0 )
        {
            std::cout << "   Modified : -";
        } else {
            time_info = gmtime(&tmp_time);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
            std::cout << "   Modified : " << buffer;
        }
        std::cout << " v0 time (" << sizeof(timev0) << "b)" << std::endl;

    } else
    {
        file.read( (char *)&timev1, 16 );

        time_t tmp_time = (time_t)(timev1.created);
        struct tm *time_info = gmtime(&tmp_time);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
        std::cout << "         Created : " << buffer;
        tmp_time = (time_t)timev1.modified;
        time_info = gmtime(&tmp_time);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
        std::cout << "   Modified : " << buffer << std::endl;
        std::cout << "   v1 time (" << sizeof(timev1) << "b)" << std::endl;
    }
}


// Utilities
//
std::string getVersionString() 
{ 
    std::string ver = "v"; 
    ver += std::to_string(s_majorVersion); ver += "."; 
    ver += std::to_string(s_minorVersion); ver += "."; 
    ver += std::to_string(s_revision);  
    return ver;
}

unsigned int swapOrder( unsigned int in )
{
    return (in >> 24) | ((in << 8) & 0x00FF0000) | ((in >> 8) & 0x0000FF00) | (in << 24);
}

unsigned short swapShort( unsigned short in )
{
    return (in >> 8) | (in << 8);
}

unsigned long swapLong( unsigned long in )
{
    return (in >> 24) | ((in << 8) & 0x00FF0000) | ((in >> 8) & 0x0000FF00) | (in << 24);
}

std::string decode_lang(uint16_t lang) {
    // Extract 5-bit values
    uint8_t char1 = (lang >> 10) & 0x1F;  // Bits 14–10
    uint8_t char2 = (lang >> 5) & 0x1F;   // Bits 9–5
    uint8_t char3 = lang & 0x1F;          // Bits 4–0

    // Convert to ASCII by adding 0x60
    std::string result;
    result += (char1 ? char1 + 0x60 : ' ');
    result += (char2 ? char2 + 0x60 : ' ');
    result += (char3 ? char3 + 0x60 : ' ');

    return result;
}