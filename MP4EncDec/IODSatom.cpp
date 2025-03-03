#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseIODSatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    // get the tag and flags, using the version structure
    //
    //  Version (1 byte): Typically 0x00, indicating the version of the IOD format.
    //
    //  Flags (3 bytes): Usually 0x000000, reserved for future use or extensions. No specific flags are defined in the standard.
    //
    struct version_t ver;
    file.read( (char *)&ver , 4 );
    std::cout   << "v" << (int)ver.version 
                << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">" 
                << ", len <" << (len-4) << ">" << std::endl;

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
        // get the IODS tag and length
        file.read( (char *)&tag, 1 );
        file.read( (char *)&desc_len, 1 );
        unsigned int real_len = (unsigned int)desc_len;
        len -= 2;

        std::cout << padding << "tag <0x" << std::hex <<  (int)tag << std::dec << ">";

        // see if we are done
        if( len <= 0 ) 
        {
            std::cout << std::endl;
            break;
        }

        if( real_len > 127 )
        {
            file.read( (char *)&alt_len, 1);
            len -= 1;
            real_len = alt_len + 128;
        }

        // get the descriptor and details
        file.read( (char *)&desc_id, 2 );
        file.read( (char *)profile, 5 );
        len -= 7;

        std::cout << ", len <" << real_len << ">, id <0x" << std::hex << (desc_id & 0x003f) << std::dec << ">";
        std::cout << ", url <" << ((desc_id & 0x0040) ? "1" : "0") << ">, inline <" << ((desc_id & 0x0080) ? "1" : "0") << ">" << std::endl;
        std::cout << padding << "profile <";
        std::cout << std::hex << "OD : 0x" << (int)profile[0] << std::dec << ", ";
        std::cout << std::hex << "scene : 0x" << (int)profile[1] << std::dec << ", ";
        std::cout << std::hex << "audio : 0x" << (int)profile[2] << std::dec << ", ";
        std::cout << std::hex << "video : 0x" << (int)profile[3] << std::dec << ", ";
        std::cout << std::hex << "graphics : 0x" << (int)profile[4] << std::dec << ", ";

        std::cout << ">" << std::endl;
    }
    m_depth--;
}