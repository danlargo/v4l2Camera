#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <map>
#include <string>
#include <cstring>

#include "wrapMP4.h"

struct mp4StreamInfo_t * mp4_info;

std::streampos tkhdDurationOffset = -1;
std::streampos mvhdDurationOffset = -1;
std::streampos mdhdDurationOffset = -1;
std::streampos mdatSizeOffset = -1;

std::streampos sttsSampleCountOffset = -1;
std::streampos sttsSampleDeltaOffset = -1;
std::streampos stszSampleCountOffset = -1;

bool writeMP4Header( std::fstream & file, struct mp4StreamInfo_t * info )
{
    // save the mp4 info for later
    mp4_info = info;

    // write the ftyp atom
    if( !writeFTYPatom( file ) ) return false;
    if( !writeFREEatom( file ) ) return false;
    if( !writeMOOVatom( file ) ) return false;

    return true;
}

bool writeFTYPatom( std::fstream & file )
{
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 'f', 't', 'y', 'p' };
        unsigned char majorBrand[4] = { 'i', 's', 'o', 'm' };
        unsigned int ver = 1;
        unsigned char compatibleBrands[16] = { 'i', 's', 'o', 'm', 'i', 's', 'o', '2', 'a', 'v', 'c', '1', 'm', 'p', '4', '2' };
    } ftyp_atom;
    #pragma pack(pop)

    // set up the structure for the atom
    ftyp_atom.size = sizeof(ftyp_atom);
    ftyp_atom.size = swapEndian(ftyp_atom.size);
    ftyp_atom.ver = swapEndian(ftyp_atom.ver);

    // write the atom data
    file.write( (char*)&ftyp_atom, sizeof(ftyp_atom) );

    std::cout << "[\033[1;34minfo\033[0m] wrote [ftyp] atom" << std::endl;

    return true;
}

bool writeFREEatom( std::fstream & file )
{
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 'f', 'r', 'e', 'e' };
        unsigned char author[80] = { 'C', 'r', 'e', 'a', 't', 'e', 'd', ' ', 'w', 'i', 't', 'h', ' ', 'w', 'r', 'a', 'p', 'M', 'P', '4', ' ', 'v', '0', '.', '1', '.', '1', '0', ',', ' ', 'b', 'y', ' ', 's', 'l', 'a', 'p', 'f', 'r', 'o', 'g', ' ', 'L', 'a', 'b', 's', ',', ' ', '(', 'c', ')', ' ', '2', '0', '2', '5', ',', ' ', 'a', 'l', 'l', ' ', 'r', 'i', 'g', 'h', 't', 's', ' ', 'r', 'e', 's', 'e', 'r', 'v', 'e', 'd', 0 };
    } free_atom;
    #pragma pack(pop)
    
    int len = sizeof(free_atom);
    int pad = 16 - (len % 16);

    // set up the size
    free_atom.size = sizeof(free_atom) + pad;
    free_atom.size = swapEndian(free_atom.size);
    
    // write the atom data
    file.write( (char*)&free_atom, len );

    // send the padding
    for( int i = 0; i < pad; i++ ) file.write( (char*)"\0", 1 );

    std::cout << "[\033[1;34minfo\033[0m] wrote [free] atom" << std::endl;

    return true;
}


bool writeMOOVatom( std::fstream & file )
{
    // grab the file position so we can re-wind and update the size
    std::streampos moovSizeOffset = file.tellp();

    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 'm', 'o', 'o', 'v' };
    } moov_atom;
    #pragma pack(pop)
    
    unsigned int real_size = sizeof(moov_atom);
    moov_atom.size = swapEndian(real_size);

    file.write( (char*)&moov_atom, real_size );

    std::cout << "[\033[1;34minfo\033[0m] writing [moov] atom" << std::endl;

    // Movie Header Atom - first - MVHD
    real_size += writeMVHDatom( file );
    // slide in the authoring info now - [UDTA][AUTH]
    real_size += writeAUTHatom( file );
    // now the important stuff
    real_size += writeTRAKatom( file );

    // update the size of the moov atom
    file.seekp( moovSizeOffset );
    unsigned int tmp = swapEndian(real_size);
    file.write( (char*)&tmp, 4 );

    // go back to the end of the file
    file.seekp( 0, std::ios::end );

    std::cout << "[\033[1;34minfo\033[0m] wrote [moov] atom" << std::endl;

    return true;
}


int writeMVHDatom( std::fstream & file )
{
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 'm', 'v', 'h', 'd' };
        unsigned char ver = 0;
        unsigned char flags[3] = { 0,0,0 };
        unsigned int creationTime;
        unsigned int modificationTime;
        unsigned int timeScale = mp4_info->timeScale_swapped;
        unsigned int duration = swapEndian( mp4_info->videoDuration);
        unsigned short rate[2] = { swapEndian((unsigned short)0x00000001), 0 };
        unsigned char volume[2] = { 0, 0 };
        unsigned char reserved[10] = { 0,0,0,0,0,0,0,0,0,0 };
        unsigned int matrix[9] = { buildINT16_2(0,1), buildINT16_2(0,0), buildINT16_2(0,0) 
                                 , buildINT16_2(0,0), buildINT16_2(0,1), buildINT16_2(0,0)
                                 , buildINT16_2(0,0), buildINT16_2(0,0), buildINT16_2(0,16384) };
        unsigned char reserved2[24];
        unsigned int nextTrackID = swapEndian((unsigned int)2);
    } mvhd_atom;
    #pragma pack(pop)

    // setup the header
    mvhd_atom.size = sizeof(mvhd_atom);
    mvhd_atom.size = swapEndian(mvhd_atom.size);

    // time stamps
    mvhd_atom.creationTime = mp4ToInt32("");            // NOW
    mvhd_atom.creationTime = swapEndian(mvhd_atom.creationTime);
    mvhd_atom.modificationTime = mp4ToInt32("");        // NOW
    mvhd_atom.modificationTime = swapEndian(mvhd_atom.modificationTime);

    // write the atom
    file.write( (char*)&mvhd_atom, sizeof(mvhd_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ..wrote [mvhd] atom" << std::endl;

    return sizeof(mvhd_atom);
}

int writeAUTHatom( std::fstream & file )
{
    // author string, needed to calc all the lengths
    std::string author = "wrapMP4 v0.1.20";

    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 'u', 'd', 't', 'a' };
        unsigned int auth_size;
        unsigned char auth_tag[4] = { 'a', 'u', 't', 'h' };
        unsigned int flags = 0;
        unsigned short lang = swapEndian(encode_lang("eng"));
        unsigned char author[16] = { 'w', 'r', 'a', 'p', 'M', 'P', '4', ' ', 'v', '0', '.', '1', '.', '2', '0', 0};
    } udta_atom;
    #pragma pack(pop)

    // setup the size of the atom
    udta_atom.size = sizeof(udta_atom);
    udta_atom.size = swapEndian(udta_atom.size);

    // setup the author atom
    udta_atom.auth_size = sizeof(udta_atom) - 8;
    udta_atom.auth_size = swapEndian(udta_atom.auth_size);

    // write the data
    file.write( (char*)&udta_atom, sizeof(udta_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ..wrote [udta/auth] atom" << std::endl;

    return sizeof(udta_atom);
}

int writeTRAKatom( std::fstream & file )
{
    // grab the file location so we can come back
    std::streampos trakSizeOffset = file.tellp();

    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 't', 'r', 'a', 'k' };
    } trak_atom;
    #pragma pack(pop)

    unsigned int real_size = sizeof(trak_atom);
    trak_atom.size = swapEndian(real_size);

    // write the atom
    file.write( (char*)&trak_atom, real_size );

    std::cout << "[\033[1;34minfo\033[0m] ..writing [trak] atom" << std::endl;

    // insert the contained atoms
    real_size += writeTKHDatom( file );
    real_size += writeMDIAatom( file );

    // update the size
    file.seekp( trakSizeOffset );
    unsigned int tmp = swapEndian(real_size);
    file.write( (char*)&tmp, 4 );

    // jump back to the end of the file
    file.seekp( 0, std::ios::end );

    std::cout << "[\033[1;34minfo\033[0m] ..wrote [trak] atom" << std::endl;

    return real_size;
}


int writeTKHDatom( std::fstream & file )
{
    // mark where the size of the atom is
    std::streampos tkhdSizeOffset = file.tellp();

    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 't', 'k', 'h', 'd' };
        unsigned char ver = 0;
        unsigned char flags[3] = { 0,0,0 };
        unsigned int creationTime;
        unsigned int modificationTime;
        unsigned int trackID = swapEndian((unsigned int)1);
        unsigned int reserved = 0;
        unsigned int duration = swapEndian( mp4_info->videoDuration );
        unsigned char reserved3[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        unsigned short layer = 0;
        unsigned short group = 0;
        unsigned short volume = 0;
        unsigned short reserved2 = 0;
        unsigned int matrix[9] = { buildINT16_2(0,1), buildINT16_2(0,0), buildINT16_2(0,0) 
                                 , buildINT16_2(0,0), buildINT16_2(0,1), buildINT16_2(0,0)
                                 , buildINT16_2(0,0), buildINT16_2(0,0), buildINT16_2(0,16384) };
        unsigned short width[2] = { swapEndian( (unsigned short)mp4_info->width ), 0 };
        unsigned short height[2] = { swapEndian( (unsigned short)mp4_info->height ), 0 };
    } tkhd_atom;
    #pragma pack(pop)

    // setup the header
    tkhd_atom.size = sizeof(tkhd_atom);
    tkhd_atom.size = swapEndian(tkhd_atom.size);

    // time stamps
    tkhd_atom.creationTime = mp4ToInt32("");
    tkhd_atom.creationTime = swapEndian(tkhd_atom.creationTime);
    tkhd_atom.modificationTime = mp4ToInt32("");
    tkhd_atom.modificationTime = swapEndian(tkhd_atom.modificationTime);

    // write the atom
    file.write( (char*)&tkhd_atom, sizeof(tkhd_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ....wrote [tkhd] atom" << std::endl;

    return sizeof(tkhd_atom);
}

int writeMDIAatom( std::fstream & file )
{
    // grab the size location for update after
    std::streampos mdiaSizeOffset = file.tellp();

    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 'm', 'd', 'i', 'a' };
    } mdia_atom;
    #pragma pack(pop)

    // setup the header
    unsigned int real_size = sizeof(mdia_atom);
    mdia_atom.size = swapEndian(real_size);

    // write the header
    file.write( (char*)&mdia_atom, sizeof(mdia_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ....writing [mdia] atom" << std::endl;

    // write contained atoms
    real_size += writeMDHDatom( file );
    real_size += writeHDLRatom( file );
    real_size += writeMINFatom( file );

    // go back and update the size
    file.seekp( mdiaSizeOffset );
    unsigned int tmp = swapEndian(real_size);
    file.write( (char*)&tmp, 4 );

    // go to end of the file
    file.seekp( 0, std::ios::end );

    std::cout << "[\033[1;34minfo\033[0m] ....wrote [mdia] atom" << std::endl;

    return real_size;
}

int writeMDHDatom( std::fstream &file )
{
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 'm', 'd', 'h', 'd' };
        unsigned char ver = 0;
        unsigned char flags[3] = { 0,0,0 };
        unsigned int creationTime;
        unsigned int modificationTime;
        unsigned int timeScale = mp4_info->timeScale_swapped;
        unsigned int duration = swapEndian( mp4_info->videoDuration );
        unsigned short lang = swapEndian(encode_lang("eng"));
        unsigned short reserved = 0;
    } mdhd_atom;
    #pragma pack(pop)

    // time stamps
    mdhd_atom.creationTime = mp4ToInt32("");
    mdhd_atom.creationTime = swapEndian(mdhd_atom.creationTime);
    mdhd_atom.modificationTime = mp4ToInt32("");
    mdhd_atom.modificationTime = swapEndian(mdhd_atom.modificationTime);

    // setup the header
    mdhd_atom.size = sizeof(mdhd_atom);
    mdhd_atom.size = swapEndian(mdhd_atom.size);

    // write the atom
    file.write( (char*)&mdhd_atom, sizeof(mdhd_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ......wrote [mdhd] atom" << std::endl;

    return sizeof(mdhd_atom);
}

int writeHDLRatom( std::fstream &file )
{
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 'h', 'd', 'l', 'r' };
        unsigned char ver = 0;
        unsigned char flags[3] = { 0,0,0 };
        unsigned int predefined = 0;
        unsigned char hdlrType[4] = { 'v', 'i', 'd', 'e' };
        unsigned char reserved2[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
        unsigned char name[12] = { 'H', '2', '6', '4', 'D', 'e', 'c', 'o', 'd', 'e', 'r', 0 };
    } hdlr_atom;
    #pragma pack(pop)

    // setup the header
    hdlr_atom.size = sizeof(hdlr_atom);
    hdlr_atom.size = swapEndian(hdlr_atom.size);

    // write the atom
    file.write( (char*)&hdlr_atom, sizeof(hdlr_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ......wrote [hdlr] atom" << std::endl;

    return sizeof(hdlr_atom);

}

int writeMINFatom( std::fstream &file )
{
    // grab the file location
    std::streampos minfSizeOffset = file.tellp();

    // create a minimal header
    #pragma pack(push, 1)
    struct
    {
        unsigned int size = 8;
        unsigned char tag[4] = { 'm', 'i', 'n', 'f' };
    } minf_atom;
    #pragma pack(pop)

    // setup the header
    unsigned int real_size = sizeof(minf_atom);
    minf_atom.size = swapEndian(real_size);

    // write the header
    file.write( (char*)&minf_atom, sizeof(minf_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ......writing [minf] atom" << std::endl;

    // write the other sections
    real_size += writeVMHDatom( file );
    real_size += writeDINFatom( file );
    real_size += writeSTBLatom( file );

    // update the size
    file.seekp( minfSizeOffset );
    unsigned int tmp = swapEndian(real_size);
    file.write( (char*)&tmp, 4 );

    // go to the end of the file
    file.seekp( 0, std::ios::end );

    std::cout << "[\033[1;34minfo\033[0m] ......wrote [minf] atom" << std::endl;

    return real_size;
}

int writeVMHDatom( std::fstream &file )
{
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 'v', 'm', 'h', 'd' };
        unsigned char ver = 0;
        unsigned char flags[3] = { 0,0,1 };
        unsigned short graphicsMode = 0;
        unsigned short opcolor[3] = { 0, 0, 0 };
    } vmhd_atom;
    #pragma pack(pop)

    // setup the header
    vmhd_atom.size = sizeof(vmhd_atom);
    vmhd_atom.size = swapEndian(vmhd_atom.size);

    // write the atom
    file.write( (char*)&vmhd_atom, sizeof(vmhd_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ........wrote [vmhd] atom" << std::endl;

    return sizeof(vmhd_atom);
}

int writeDINFatom( std::fstream &file )
{
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 'd', 'i', 'n', 'f' };
        unsigned int drefSize;
        unsigned char drefTag[4] = { 'd', 'r', 'e', 'f' };
        unsigned char ver = 0;
        unsigned char flags[3] = { 0,0,0 };
        unsigned int entryCount = swapEndian((unsigned int)1);
        unsigned int urlSize;
        unsigned char urlTag[4] = { 'u', 'r', 'l', ' ' };
        unsigned char ver2 = 0;
        unsigned char flags2[3] = { 0,0,1 };
    } dinf_atom;
    #pragma pack(pop)

    // setup the header
    dinf_atom.size = sizeof(dinf_atom);
    dinf_atom.size = swapEndian(dinf_atom.size);    

    // setup the dref
    dinf_atom.drefSize = sizeof(dinf_atom) - 8;
    dinf_atom.drefSize = swapEndian(dinf_atom.drefSize);

    // setup the url
    dinf_atom.urlSize = 12;
    dinf_atom.urlSize = swapEndian(dinf_atom.urlSize);

    // write the atom
    file.write( (char*)&dinf_atom, sizeof(dinf_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ........wrote [dinf] atom" << std::endl;

    return sizeof(dinf_atom);

}

int writeSTBLatom( std::fstream &file )
{
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 's', 't', 'b', 'l' };
    } stbl_atom;
    #pragma pack(pop)

    // grab the file location
    std::streampos stblSizeOffset = file.tellp();

    // setup the header
    unsigned int real_size = sizeof(stbl_atom);
    stbl_atom.size = swapEndian(real_size);

    // write the atom
    file.write( (char*)&stbl_atom, sizeof(stbl_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ........writing [stbl] atom" << std::endl;

    real_size += writeSTSDatom( file );
    real_size += writeSTTSatom( file );
    real_size += writeSTSCatom( file );
    real_size += writeSTSZatom( file );
    real_size += writeSTCOatom( file );

    // update the size
    file.seekp( stblSizeOffset );
    unsigned int tmp = swapEndian(real_size);
    file.write( (char*)&tmp, 4 );

    // go to the end of the file
    file.seekp( 0, std::ios::end );

    std::cout << "[\033[1;34minfo\033[0m] ........wrote [stbl] atom" << std::endl;

    return real_size;
}

int writeSTSDatom( std::fstream &file )
{
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 's', 't', 's', 'd' };
        unsigned char ver = 0;
        unsigned char flags[3] = { 0,0,0 };
        unsigned int entries = swapEndian((unsigned int)1);
        struct
        {
            unsigned int size;
            unsigned char tag2[4] = { 'a', 'v', 'c', '1' };
            unsigned char reserved[6] = { 0,0,0,0,0,0 };
            unsigned short dataRefIndex = swapEndian((unsigned short)1);
            unsigned char preDefined[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
            unsigned short width = swapEndian((unsigned short)mp4_info->width);
            unsigned short height = swapEndian((unsigned short)mp4_info->height);
            unsigned short horizRes[2] = { swapEndian((unsigned short)72), 0 };
            unsigned short vertRes[2] = { swapEndian((unsigned short)72), 0 };
            unsigned int reserved2 = 0;
            unsigned short frameCount = swapEndian((unsigned short)1);
            unsigned char compressor[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
            unsigned short depth = swapEndian((unsigned short)24);
            unsigned short reserved3 = swapEndian((unsigned short)0xffff);
            struct
            {
                unsigned int size;
                unsigned char tag3[4] = { 'a', 'v', 'c', 'C' };
                unsigned char ver2 = 0x01;
                unsigned char sps_profile = mp4_info->sps[1];
                unsigned char sps_compat = mp4_info->sps[2];
                unsigned char sps_level = mp4_info->sps[3];
                unsigned char sps_nalu = 0xff;
            } avcc_box;
        } avc1_box;
    } stsd_atom;
    #pragma pack(pop)
    
    // get size of SPS and PPS data
    unsigned int sps_size = 1 + 2 + mp4_info->sps_size;
    unsigned int pps_size = 1 + 2 + mp4_info->pps_size;

    // setup the header
    stsd_atom.size = sizeof(stsd_atom) + sps_size + pps_size + 4;
    stsd_atom.avc1_box.size = sizeof(stsd_atom.avc1_box) + sps_size + pps_size + 4;
    stsd_atom.avc1_box.avcc_box.size = sizeof(stsd_atom.avc1_box.avcc_box) + sps_size + pps_size + 4;
    stsd_atom.size = swapEndian(stsd_atom.size);
    stsd_atom.avc1_box.size = swapEndian(stsd_atom.avc1_box.size);
    stsd_atom.avc1_box.avcc_box.size = swapEndian(stsd_atom.avc1_box.avcc_box.size);

    // write the atom
    file.write( (char*)&stsd_atom, sizeof(stsd_atom) );

    // insert the SPS and PPS data
    //
    // SPS count
    //
    unsigned char spsCount = 0xe1;
    file.write( (char*)&spsCount, 1 );
    // SPS size
    unsigned short spsSize = swapEndian(mp4_info->sps_size);
    file.write( (char*)&spsSize, 2 );
    // SPS data
    file.write( (char*)mp4_info->sps, mp4_info->sps_size );

    // PPS count
    unsigned char ppsCount = 0x01;
    file.write( (char*)&ppsCount, 1 );
    // PPS size
    unsigned short ppsSize = swapEndian(mp4_info->pps_size);
    file.write( (char*)&ppsSize, 2 );
    // PPS data
    file.write( (char*)mp4_info->pps, mp4_info->pps_size );

    // add in 4 bytes chromo, bit depth luma and bit depth chroma, and SPS extended size, all 0
    unsigned int zero = 0;
    file.write( (char*)&zero, 4 );

    std::cout << "[\033[1;34minfo\033[0m] ..........wrote [stsd] atom" << std::endl;

    return sizeof(stsd_atom) + sps_size + pps_size + 4;
}

int writeSTTSatom( std::fstream &file )
{
    // Time to Sample Atom
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 's', 't', 't', 's' };
        unsigned char ver = 0;
        unsigned char flags[3] = { 0,0,0 };
        unsigned int entryCount = swapEndian((unsigned int)1);
        unsigned int sampleCount = swapEndian((unsigned int)mp4_info->frame_count);
        unsigned int sampleDelta = swapEndian((unsigned int)mp4_info->ticks_per_frame);
    } stts_atom;
    #pragma pack(pop)

    // setup the header
    stts_atom.size = sizeof(stts_atom);
    stts_atom.size = swapEndian(stts_atom.size);

    // write the atom
    file.write( (char*)&stts_atom, sizeof(stts_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ..........wrote [stts] atom" << std::endl;

    return sizeof(stts_atom);
}

int writeSTSCatom( std::fstream &file )
{
    // Sample to Chunk ATOM, keep it simple, the frames are all the same
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 's', 't', 's', 'c' };
        unsigned char ver = 0;
        unsigned char flags[3] = { 0,0,0 };
        unsigned int entryCount = swapEndian((unsigned int)1);
        unsigned int firstChunk = swapEndian((unsigned int)1);
        unsigned int samplesPerChunk = swapEndian((unsigned int)mp4_info->frame_count);
        unsigned int sampleDescIndex = swapEndian((unsigned int)1);
    } stsc_atom;
    #pragma pack(pop)
    
    // setup the header
    stsc_atom.size = sizeof(stsc_atom);
    stsc_atom.size = swapEndian(stsc_atom.size);

    // write the atom
    file.write( (char*)&stsc_atom, sizeof(stsc_atom) );

    std::cout << "[\033[1;34minfo\033[0m] ..........wrote [stsc] atom" << std::endl;

    return sizeof(stsc_atom);
}

int writeSTSZatom( std::fstream &file )
{
    // Sample Size ATOM
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 's', 't', 's', 'z' };
        unsigned char ver = 0;
        unsigned char flags[3] = { 0,0,0 };
        unsigned int sampleSize = 0;
        unsigned int sampleCount = swapEndian((unsigned int)mp4_info->frame_count);
    } stsz_atom;
    #pragma pack(pop)

    // setup the header
    stsz_atom.size = sizeof(stsz_atom) + (4*mp4_info->frame_count);
    stsz_atom.size = swapEndian(stsz_atom.size);

    // write the atom
    file.write( (char*)&stsz_atom, sizeof(stsz_atom) );

    // sample table
    for( int i = 0; i < mp4_info->frame_count; i++ )
    {
        unsigned int sampleSize = swapEndian(mp4_info->frame_sizes[i]);
        file.write( (char*)&sampleSize, 4 );
    }

    std::cout << "[\033[1;34minfo\033[0m] ..........wrote [stsz] atom, frame size table, frames : " << mp4_info->frame_count << std::endl;

    return sizeof(stsz_atom) + (4*mp4_info->frame_count);
}

int writeSTCOatom( std::fstream &file )
{
    // Chunk Offset ATOM
    #pragma pack(push, 1)
    struct
    {
        unsigned int size;
        unsigned char tag[4] = { 's', 't', 'c', 'o' };
        unsigned char ver = 0;
        unsigned char flags[3] = { 0,0,0 };
        unsigned int entryCount = swapEndian((unsigned int)1);
        //unsigned int chunkOffset = swapEndian((unsigned int)mdatSizeOffset + 8);
    } stco_atom;
    #pragma pack(pop)
    
    // setup the header
    stco_atom.size = sizeof(stco_atom) + 4;
    stco_atom.size = swapEndian(stco_atom.size);

    // write the atom
    file.write( (char*)&stco_atom, sizeof(stco_atom) );

    // write the chunk offset
    unsigned int here = file.tellp();
    here = here + 4 + 8;        // jump over this integer and the MDAT header
    unsigned int chunkOffset = swapEndian(here);

    file.write( (char*)&chunkOffset, 4 );

    std::cout << "[\033[1;34minfo\033[0m] ..........wrote [stco] atom" << std::endl;

    return sizeof(stco_atom) + 4;
}


bool writeMDATatom( std::ifstream &inFile, std::fstream & outFile )
{
    #pragma pack(push, 1)
    struct
    {
        unsigned int size = mp4_info->mdatSize + 8;
        unsigned char tag[4] = { 'm', 'd', 'a', 't' };
    } mdat_atom;
    #pragma pack(pop)

    unsigned int size_check = 0;

    // setup the header
    mdat_atom.size = swapEndian(mdat_atom.size);

    // write the atom
    outFile.write( (char*)&mdat_atom, sizeof(mdat_atom) );

    std::cout << "[\033[1;34minfo\033[0m] writing [mdat] atom" << std::endl;

    // read the input file and write the data
    int i = 0;
    while( !inFile.eof() )
    {
        // read the frame header
        struct h264FrameHeader_t * frame = getFrameHeader( inFile );

        if( frame == nullptr ) break;

        // grab the frame
        char * buf = new char[frame->frame_size];
        inFile.read( buf, frame->frame_size );
        
        // figure out where h264 data frame starts
        unsigned int data_start = frame->frame_offset + 4;            // skip over 00 00 00 01 tag
        unsigned int data_size = frame->frame_size - data_start;

        // compare to the save frame sizes in the STSZ black, which include the 00 00 00 01 tag
        if( data_size != mp4_info->frame_sizes[i++]-4 )
        {
            std::cout << "[\033[0;31mwarning\033[0m] MDAT size mismatch : frame " << (i-1) << " : scan size = " << mp4_info->frame_sizes[i]-4 << ", copied bytes = " << data_size << ", exiting" << std::endl;
            return false;
        }
        // write the frame size
        unsigned int tmp = swapEndian(data_size);
        outFile.write( (char*)&tmp, 4 );

        // write the frame
        outFile.write( (char*)&buf[data_start], data_size );

        // update the amount of data written
        size_check += data_size + 4;

        // free the memory and do it again
        delete frame;
        delete buf;
    }

    // check if the sizes match
    if( size_check != mp4_info->mdatSize )
    {
        std::cout << "[\033[0;31mwarning\033[0m] MDAT size mismatch : scan size = " << mp4_info->mdatSize << ", copied bytes = " << size_check << std::endl;
        return false;
    }

    std::cout << "[\033[1;34minfo\033[0m] wrote [mdat] atom, frame data : total bytes = " << mp4_info->mdatSize << std::endl;

    return true;
}


bool verifyMP4Data( std::fstream & file, struct mp4StreamInfo_t * info, int mdatStartLocation )
{
    // rewind the file to the beginning
    file.seekg( 0, std::ios::beg );

    std::cout << "[\033[1;34minfo\033[0m] verifying MP4 data sizes, jumping to [mdat] atom" << std::endl;

    // jump to the start of the MDAT region
    file.seekg( mdatStartLocation, std::ios::beg );

    // read the tag and the size
    unsigned int mdatSize = 0;
    char tag[4] = { 0,0,0,0 };
    file.read( (char*)&mdatSize, 4 );
    file.read( tag, 4 );
    mdatSize = swapEndian(mdatSize);

    // check the tag
    if( tag[0] == 'm' && tag[1] == 'd' && tag[2] == 'a' && tag[3] == 't' )
    {
        int sizeCnt = mdatSize-8;
        std::cout << "[\033[1;34minfo\033[0m] ..found [mdat] atom, size = " << mdatSize << ", data size = " << sizeCnt << ", walking the data" << std::endl;

        // walk the data
        int i = 0;
        std::cout << "[\033[1;34minfo\033[0m] ..comparing stsz size table to file data" << std::endl;

        while( !file.eof() && sizeCnt > 0)
        {
            // read the frame size
            unsigned int frame_size = 0;
            file.read( (char*)&frame_size, 4 );
            frame_size = swapEndian(frame_size);
            sizeCnt -= 4;

            // read the frame data
            char * buf = new char[frame_size];
            file.read( buf, frame_size );
            sizeCnt -= frame_size;

            // check the frame size
            if( frame_size != info->frame_sizes[i++]-4 )
            {
                std::cout << "[\033[0;31merror\033[0m] ..frame size mismatch : frame " << (i-1) << " : scan size = " << info->frame_sizes[i]-4 << ", copied bytes = " << frame_size << ", exiting" << std::endl;
                return false;
            }

            // free the memory and do it again
            delete buf;

            // check the size
            if( sizeCnt < 0 )
            {
                std::cout << "[\033[0;31merror\033[0m] ..MDAT size mismatch, MDAT count is zero and NOT EOF" << std::endl;
                return false;
            }
        }
    }
    else
    {
        std::cout << "[\033[0;31merror\033[0m] ..expected [mdat] atom, found [" << tag[0] << tag[1] << tag[2] << tag[3] << "] atom" << std::endl;
        return false;
    }

    std::cout << "[\033[1;34minfo\033[0m] verified [mdat] atom" << std::endl;

    return true;
}
