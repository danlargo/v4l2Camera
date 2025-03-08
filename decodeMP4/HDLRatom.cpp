#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseHDLRatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

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
    char * name = new char[len - sizeof(hdlr_atom_t) + 1];

    file.read( (char *)&hdlr_atom, sizeof(hdlr_atom_t) );
    file.read( (char *)name, len - sizeof(hdlr_atom_t) );
    name[len - sizeof(hdlr_atom_t)] = 0;

    // clear the name
    std::string tmp = name;
    delete [] name;
    trim(tmp);

    // determine the handler type
    std::string handler = "";
    handler += hdlr_atom.h_str[0];
    handler += hdlr_atom.h_str[1];
    handler += hdlr_atom.h_str[2];
    handler += hdlr_atom.h_str[3];

    std::string h_ext = "";
    if( handler == "soun" ) h_ext = "Audio";
    if( handler == "vide" ) h_ext = "Video";
    if( handler == "subt" ) h_ext = "Subtitle";
    if( handler == "text" ) h_ext = "Text";
    if( handler == "hint" ) h_ext = "Hint";
    if( handler == "meta" ) h_ext = "Metadata";
    if( handler == "tmcd" ) h_ext = "Timecode";
    if( handler == "mdta" ) h_ext = "Metadata";

    // display the data
    if( tmp.length() > 0 ) std::cout << "\033[1;36m" << tmp << "\033[0m, ";
    std::cout << "type <\033[1;36m" << handler << "\033[0m> " << h_ext << ", ";
    std::cout << "0x" << std::hex << swapOrder(hdlr_atom.h_int) << std::dec << ", ";
    std::cout << "hdlr v" << (int)hdlr_atom.version << ", flags <" << std::hex << (int)hdlr_atom.flags[0] << (int)hdlr_atom.flags[1] << (int)hdlr_atom.flags[2] << std::dec << ">, " << std::endl;

    m_depth--;
}