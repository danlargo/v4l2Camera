#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

// known MP4 variants we can parse
//
const int num_known_mp4 = 5;
struct known_variant_t known_mp4[num_known_mp4] = 
{ 
    {"isom", -1}, 
    {"mp42",-1},
    {"mp41",-1},
    {"qt  ",0},
    {"3gp4", 0x300}
};

// FTYP atom
//
bool parseFTYPatom( std::ifstream &file, unsigned long len )
{
    m_depth++;

    bool ret = false;
    //
    // FTYP header format

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
    
    std::cout << "MP4 Variant <\033[1;36m" << major_brand << " v" << std::hex << swapOrder(ftyp.minorVersion) << std::dec << "\033[0m";

    // read the compatible brands, if they are appended
    if( len > 8 )
    {
        char * buffer = new char[len-8];
        file.read( buffer, len-8 );

        // display the compatible brands
        std::cout << ">   aka < ";
        for( int i = 0; i < (len-8); i+=4 ) std::cout << buffer[i] << buffer[i+1] << buffer[i+2] << buffer[i+3] << " ";
        std::cout << ">" << std::endl;

        delete [] buffer;
    }

    // add a blank line
    std::cout << std::endl;

    // check if we know this variant
    if( !checkKnownVariant( major_brand, swapOrder(ftyp.minorVersion) ) )
    {
        std::cout   << std::endl << std::endl << "[\x1b[1;31mwarning\x1b[0m] : [" << major_brand <<  " v" << std::hex << swapOrder(ftyp.minorVersion)
                    << std::dec << "] no parsing information for this MP4 variant, exiting" << std::endl;

    } else ret = true;

    m_depth--;
    return ret;
}


bool checkKnownVariant( std::string name, int ver )
{
    bool ret = false;

    for( int i = 0; i < num_known_mp4; i++ )
    {
        if( (known_mp4[i].name == name) )
        {
            // use -1 as catch all
            if( (known_mp4[i].ver == ver) || (known_mp4[i].ver == -1) )
            {
                ret = true;
                break;
            }
        }
    }

    return ret;
}

