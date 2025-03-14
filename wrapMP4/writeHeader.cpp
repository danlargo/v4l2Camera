#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <map>
#include <string>

#include "wrapMP4.h"

unsigned int timeScale = 1000;
unsigned int timeScale_swapped;
unsigned int video_width = 0;
unsigned int video_height = 0;
unsigned int videoDuration = 0;
unsigned int mdatSize = 0;

std::streampos tkhdDurationOffset = -1;
std::streampos mvhdDurationOffset = -1;
std::streampos mdatSizeOffset = -1;
std::streampos mdhdDurationOffset = -1;

bool writeMP4Header( std::ofstream & file, int width, int height )
{
    // fix up the globals that are needed
    video_width = width;
    video_height = height;
    timeScale_swapped = swapEndian(timeScale);

    // write the ftyp atom
    if( !writeFTYPatom( file ) ) return false;
    if( !writeFREEatom( file ) ) return false;
    if( !writeMOOVatom( file ) ) return false;
    if( !writeMDATatom( file ) ) return false;

    return true;
}

bool writeFTYPatom( std::ofstream & file )
{
    // setup the header
    unsigned int size = 32;
    unsigned char tag[4] = { 'f', 't', 'y', 'p' };
    unsigned char majorBrand[4] = { 'i', 's', 'o', 'm' };
    unsigned int ver = 1;
    unsigned char compatibleBrands[16] = { 'i', 's', 'o', 'm', 'i', 's', 'o', '2', 'a', 'v', 'c', '1', 'm', 'p', '4', '2' };

    // write the header
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );
    file.write( (char*)&majorBrand, 4 );
    tmp = swapEndian(ver);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&compatibleBrands, 16 );

    return true;
}

bool writeFREEatom( std::ofstream & file )
{
    // author string, needed to calc all the lengths
    std::string author = "Created with wrapMP4 v0.1.10, by slapfrog Labs, (c) 2025, all rights reserved";
    int len = author.length();

    // pad out to 16 byte boundary, add all 16 if there is zero padding
    int pad = 16 - (len % 16);

    // write the header
    unsigned int size = len + 8 + pad;
    size = swapEndian(size);
    unsigned char tag[4] = { 'f', 'r', 'e', 'e' };
    file.write( (char*)&size, 4 );
    file.write( (char*)&tag, 4 );

    // write the author string
    file.write( author.c_str(), len );
    // send the padding
    for( int i = 0; i < pad; i++ ) file.write( (char*)"\0", 1 );

    return true;
}


bool writeMOOVatom( std::ofstream & file )
{
    // grab the file position so we can re-wind and update the size
    std::streampos moovSizeOffset = file.tellp();

    // write the header
    unsigned int size = 8;
    unsigned char tag[4] = { 'm', 'o', 'o', 'v' };

    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    // Movie Header Atom - first - MVHD
    size += writeMVHDatom( file );

    // slide in the authoring info now - [UDTA][AUTH]
    size += writeAUTHatom( file );

    // now the important stuff
    size += writeTRAKatom( file );

    // update the size
    file.seekp( moovSizeOffset );
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );

    // go to the end of the file now
    file.seekp( 0, std::ios::end );

    return true;
}


int writeMVHDatom( std::ofstream & file )
{
    // remember the file location so we can update the duration later
    int mvhdSizeOffset = file.tellp();

    // setup the header
    unsigned int size = 8;

    unsigned char tag[4] = { 'm', 'v', 'h', 'd' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    unsigned char ver = 0;
    unsigned char flags[3] = { 0,0,0 };
    file.write( (char*)&ver, 1 );
    file.write( (char*)&flags, 3 );
    size += 4;

    // time stamps
    unsigned int creationTime = mp4ToInt32("");
    creationTime = swapEndian(creationTime);
    unsigned int modificationTime = mp4ToInt32("");
    modificationTime = swapEndian(modificationTime);
    file.write( (char*)&creationTime, 4 );
    file.write( (char*)&modificationTime, 4 );
    size += 8;
    
    // time scale and duration
    file.write( (char*)&timeScale_swapped, 4 );

    // grab  the file location of the duration so we can update it later
    mvhdDurationOffset = file.tellp();
    tmp = swapEndian(videoDuration);
    file.write( (char*)&tmp, 4 );
    size += 8;

    unsigned short rate[2] = { swapEndian((unsigned short)0x00000001), 0 };
    unsigned char volume[2] = { 0, 0 };
    file.write( (char*)&rate, 4 );
    file.write( (char*)&volume, 2 );
    size += 6;

    // output padding
    unsigned char reserved[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    file.write( (char*)&reserved, 10 );
    size += 10;

    // transform matrix
    unsigned int matrix[9] = { buildINT16_2(0,1), buildINT16_2(0,0), buildINT16_2(0,0) 
                             , buildINT16_2(0,0), buildINT16_2(0,1), buildINT16_2(0,0)
                             , buildINT16_2(0,0), buildINT16_2(0,0), buildINT16_2(0,16384) };
    // output the elements one at a time
    for( int i = 0; i < 9; i++ ) 
    {
        unsigned int tmp2 = matrix[i];
        file.write( (char*)&tmp2, 4 );
    }
    size += 36;

    // output padding
    file.write( (char*)&reserved, 24 );
    size += 24;

    // next track ID, assuming we ony have one track
    unsigned int nextTrackID = 2;
    nextTrackID = swapEndian(nextTrackID);
    file.write( (char*)&nextTrackID, 4 );
    size += 4;

    // now update the size
    file.seekp( mvhdSizeOffset );
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );

    // go back to the end of the file
    file.seekp( 0, std::ios::end );

    return size;
}

int writeTRAKatom( std::ofstream & file )
{
    // grab the file location so we can come back
    std::streampos trakSizeOffset = file.tellp();

    // provide an empty atom for now
    unsigned int size = 8;
    unsigned char tag[4] = { 't', 'r', 'a', 'k' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    size += writeTKHDatom( file );
    size += writeMDIAatom( file );

    // update the size
    file.seekp( trakSizeOffset );
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );

    // jump back to the end of the file
    file.seekp( 0, std::ios::end );

    return size;
}


int writeTKHDatom( std::ofstream & file )
{
    // mark where the size of the atom is
    std::streampos tkhdSizeOffset = file.tellp();

    // setup the header
    unsigned int size = 8;

    unsigned char tag[4] = { 't', 'k', 'h', 'd' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    unsigned char ver = 0;
    unsigned char flags[3] = { 0,0,0 };
    file.write( (char*)&ver, 1 );
    file.write( (char*)&flags, 3 );
    size += 4;

    // time stamps
    unsigned int creationTime = mp4ToInt32("");
    creationTime = swapEndian(creationTime);
    unsigned int modificationTime = mp4ToInt32("");
    modificationTime = swapEndian(modificationTime);
    file.write( (char*)&creationTime, 4 );
    file.write( (char*)&modificationTime, 4 );
    size += 8;

    // track ID and reserved
    unsigned int trackID = 1;
    trackID = swapEndian(trackID);
    unsigned int reserved = 0;
    reserved = swapEndian(reserved);
    file.write( (char*)&trackID, 4 );
    file.write( (char*)&reserved, 4 );
    size += 8;

    // duration
    // grab duration location in file
    tkhdDurationOffset = file.tellp();
    unsigned int duration = 0;
    duration = swapEndian(duration);
    file.write( (char*)&duration, 4 );
    size += 4;

    // output padding
    unsigned char reserved3[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    file.write( (char*)&reserved3, 8 );
    size += 8;

    // layer and group
    unsigned short layer = 0;
    layer = swapEndian(layer);
    unsigned short group = 0;
    group = swapEndian(group);
    file.write( (char*)&layer, 2 );
    file.write( (char*)&group, 2 );
    size += 4;

    // volume and reserved
    unsigned short volume = 0;
    volume = swapEndian(volume);
    unsigned short reserved2 = 0;
    reserved2 = swapEndian(reserved2);
    file.write( (char*)&volume, 2 );
    file.write( (char*)&reserved2, 2 ); 
    size += 4;

    // matrix
    unsigned int matrix[9] = { buildINT16_2(0,1), buildINT16_2(0,0), buildINT16_2(0,0) 
                             , buildINT16_2(0,0), buildINT16_2(0,1), buildINT16_2(0,0)
                             , buildINT16_2(0,0), buildINT16_2(0,0), buildINT16_2(0,16384) };
    // output the elements one at a time
    for( int i = 0; i < 9; i++ ) 
    {
        unsigned int tmp2 = matrix[i];
        file.write( (char*)&tmp2, 4 );
    }
    size += 36;

    // video width and height
    unsigned int width = buildINT16_2( 0, (unsigned short)(video_width) );
    unsigned int height = buildINT16_2( 0, (unsigned short)(video_height) );
    file.write( (char*)&width, 4 );
    file.write( (char*)&height, 4 );
    size += 8;

    // now go back and update the size of the atom
    file.seekp( tkhdSizeOffset );
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );

    // go back to end of the file
    file.seekp( 0, std::ios::end );

    return size;
}

int writeMDIAatom( std::ofstream & file )
{
    // grab the size location for update after
    std::streampos mdiaSizeOffset = file.tellp();

    // provide an empty atom for now
    unsigned int size = 8;
    unsigned char tag[4] = { 'm', 'd', 'i', 'a' };

    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );
    size += 8;

    // write contained atoms
    size += writeMDHDatom( file );
    size += writeHDLRatom( file );
    size += writeMINFatom( file );

    // go back and update the size
    file.seekp( mdiaSizeOffset );
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );

    // go to end of the file
    file.seekp( 0, std::ios::end );

    return size;
}

int writeMDHDatom( std::ofstream &file )
{
    // grab the location for the size to update later
    std::streampos mdhdSizeOffset = file.tellp();

    // setup the header
    unsigned int size = 8;
    unsigned char tag[4] = { 'm', 'd', 'h', 'd' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    unsigned char ver = 0;
    unsigned char flags[3] = { 0,0,0 };
    file.write( (char*)&ver, 1 );
    file.write( (char*)&flags, 3 );
    size += 4;

    // time stamps
    unsigned int creationTime = mp4ToInt32("");
    creationTime = swapEndian(creationTime);
    unsigned int modificationTime = mp4ToInt32("");
    modificationTime = swapEndian(modificationTime);
    file.write( (char*)&creationTime, 4 );
    file.write( (char*)&modificationTime, 4 );
    size += 8;

    // time scale and duration
    file.write( (char*)&timeScale_swapped, 4 );
    // grab the duration file location for update later
    mdhdDurationOffset = file.tellp();

    tmp = swapEndian(videoDuration);
    file.write( (char*)&tmp, 4 );
    size += 8;

    // language
    unsigned short lang = encode_lang("eng");
    lang = swapEndian(lang);
    file.write( (char*)&lang, 2 );
    size += 2;

    // go back and update the size
    file.seekp( mdhdSizeOffset );
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );

    // go to the end of the file
    file.seekp( 0, std::ios::end );

    return size;
}

int writeHDLRatom( std::ofstream &file )
{
    // grab the file location
    std::streampos hdlrSizeOffset = file.tellp();

    // size and tag
    unsigned int size = 8;
    unsigned char tag[4] = { 'h', 'd', 'l', 'r' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    // flags
    unsigned int flags = 0;
    flags = swapEndian(flags);
    file.write( (char*)&flags, 4 );
    size += 4;

    // reserved
    unsigned char reserved[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
    file.write( (char*)&reserved, 4 );
    size += 4;

    // handler type
    unsigned char hdlrType[4] = { 'a', 'v', 'c', '1' };
    file.write( (char*)&hdlrType, 4 );
    size += 4;

    // reserved
    file.write( (char*)&reserved, 12 );
    size += 12;

    // name
    std::string name = "H264Decoder ";
    name[name.length()] = 0;
    file.write( name.c_str(), name.length()+1 );

    size += name.length() + 1;

    // go back and update the size
    file.seekp( hdlrSizeOffset );
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );

    // go to the end of the file
    file.seekp( 0, std::ios::end );

    return size;

}

int writeMINFatom( std::ofstream &file )
{
    // grab the file location
    std::streampos minfSizeOffset = file.tellp();

    // size and tag
    unsigned int size = 8;
    unsigned char tag[4] = { 'm', 'i', 'n', 'f' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    // write the other sections
    size += writeVMHDatom( file );
    size += writeDINFatom( file );
    size += writeSTBLatom( file );

    // update the size
    file.seekp( minfSizeOffset );
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );

    // go to the end of the file
    file.seekp( 0, std::ios::end );

    return size;
}

int writeVMHDatom( std::ofstream &file )
{
    // create a minimal header
    unsigned int size = 20;
    unsigned char tag[4] = { 'v', 'm', 'h', 'd' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    // version and flags
    unsigned char ver = 0;
    unsigned char flags[3] = { 0,0,1 };
    file.write( (char*)&ver, 1 );
    file.write( (char*)&flags, 3 );

    // graphics mode
    unsigned short graphicsMode = 0;
    file.write( (char*)&graphicsMode, 2 );

    // opcolor
    unsigned short opcolor[3] = { 0, 0, 0 };
    for( int i = 0; i < 3; i++ ) file.write( (char*)&opcolor[i], 2 );

    return size;
    
}

int writeDINFatom( std::ofstream &file )
{
    // create a minial header, media data source, internal URL
    unsigned int size = 36;
    unsigned char tag[4] = { 'd', 'i', 'n', 'f' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    // create the dref atom
    size = 28;
    unsigned char tag2[4] = { 'd', 'r', 'e', 'f' };
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag2, 4 );

    // version and flags
    unsigned char ver = 0;
    unsigned char flags[3] = { 0,0,0 };
    file.write( (char*)&ver, 1 );
    file.write( (char*)&flags, 3 );

    // entry count
    unsigned int entryCount = 1;
    entryCount = swapEndian(entryCount);
    file.write( (char*)&entryCount, 4 );

    // create the url atom
    size = 12;
    unsigned char tag3[4] = { 'u', 'r', 'l', ' ' };
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag3, 4 );

    // version and flags
    ver = 0;
    flags[0] = 0;
    flags[1] = 0;
    flags[2] = 1;
    file.write( (char*)&ver, 1 );
    file.write( (char*)&flags, 3 );

    // reset the size
    size = 36;

    return size;

}

int writeSTBLatom( std::ofstream &file )
{
    // grab the file location
    std::streampos stblSizeOffset = file.tellp();

    // create a minimal atom
    unsigned int size = 8;
    unsigned char tag[4] = { 's', 't', 'b', 'l' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    size += writeSTSDatom( file );
    size += writeSTTSatom( file );
    size += writeSTSCatom( file );
    size += writeSTSZatom( file );
    size += writeSTCOatom( file );

    // update the size
    file.seekp( stblSizeOffset );
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );

    // go to the end of the file
    file.seekp( 0, std::ios::end );

    return size;
}

int writeSTSDatom( std::ofstream &file )
{
    // grab the file location
    std::streampos stsdSizeOffset = file.tellp();

    // write the atom header
    unsigned int size = 8;
    unsigned char tag[4] = { 's', 't', 's', 'd' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    // write the flags
    unsigned int flags = 0;
    flags = swapEndian(flags);
    file.write( (char*)&flags, 4 );
    size += 4;

    // write the number of entries
    unsigned int entries = 1;
    entries = swapEndian(entries);
    file.write( (char*)&entries, 4 );
    size += 4;

    unsigned char tag2[4] = { 'a', 'v', 'c', '1' };
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag2, 4 );
    size += 8;

    // reserved
    unsigned int reserved = 0;
    reserved = swapEndian(reserved);
    file.write( (char*)&reserved, 4 );
    size += 4;

    // reserved
    unsigned short reserved2 = 0;
    reserved2 = swapEndian(reserved2);
    file.write( (char*)&reserved2, 2 );
    size += 2;

    // data reference index
    unsigned short dataRefIndex = 1;
    dataRefIndex = swapEndian(dataRefIndex);
    file.write( (char*)&dataRefIndex, 2 );
    size += 2;

    // pre-defined
    unsigned int preDefined[3] = { 0, 0, 0 };
    for( int i = 0; i < 3; i++ ) file.write( (char*)&preDefined[i], 4 );
    size += 12;

    // width and height
    unsigned short width = video_width;
    width = swapEndian(width);
    unsigned short height = video_height;
    height = swapEndian(height);
    file.write( (char*)&width, 2 );
    file.write( (char*)&height, 2 );
    size += 4;

    // horiz and vert resolution
    unsigned int horizRes = 0x00480000;
    horizRes = swapEndian(horizRes);
    unsigned int vertRes = 0x00480000;
    vertRes = swapEndian(vertRes);
    file.write( (char*)&horizRes, 4 );
    file.write( (char*)&vertRes, 4 );
    size += 8;

    // update the size
    file.seekp( stsdSizeOffset );
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );

    // go to the end of the file
    file.seekp( 0, std::ios::end );

    return size;
}

int writeSTTSatom( std::ofstream &file )
{
    // write the atom header
    unsigned int size = 8;
    unsigned char tag[4] = { 's', 't', 't', 's' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );


    return size;
}

int writeSTSCatom( std::ofstream &file )
{
    // write the atom header
    unsigned int size = 8;
    unsigned char tag[4] = { 's', 't', 's', 'c' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );


    return size;
}

int writeSTSZatom( std::ofstream &file )
{
    // write the atom header
    unsigned int size = 8;
    unsigned char tag[4] = { 's', 't', 's', 'z' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );


    return size;
}

int writeSTCOatom( std::ofstream &file )
{
    // write the atom header
    unsigned int size = 8;
    unsigned char tag[4] = { 's', 't', 'c', 'o' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );


    return size;
}


int writeAUTHatom( std::ofstream & file )
{
    // author string, needed to calc all the lengths
    std::string author = "wrapMP4  " + getVersionString();
    int len = author.length();

    // this is in the [UDTA] [META] [AUTH] atom
    //
    // UDTA atom
    unsigned int size = 22 + len;
    unsigned char tag[4] = { 'u', 'd', 't', 'a' };
    unsigned int tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag, 4 );

    // AUTH atom
    size = 14 + len;
    unsigned char tag3[4] = { 'a', 'u', 't', 'h' };
    tmp = swapEndian(size);
    file.write( (char*)&tmp, 4 );
    file.write( (char*)&tag3, 4 );

    // flags for auth atom
    unsigned int flags = 0;
    flags = swapEndian(flags);
    file.write( (char*)&flags, 4 );

    // language code
    unsigned short lang = encode_lang("eng");
    lang = swapEndian(lang);
    file.write( (char*)&lang, 2 );

    // authoring information
    file.write( author.c_str(), author.length() );

    // reset the size
    size = 22 + len;

    return size;
}

bool writeMDATatom( std::ofstream & file )
{
    // grab the file offset at the current location
    mdatSizeOffset = file.tellp();

    // write the header
    unsigned int size = 8;
    size = swapEndian(size);
    unsigned char tag[4] = { 'm', 'd', 'a', 't' };
    file.write( (char*)&size, 4 );
    file.write( (char*)&tag, 4 );

    return true;
}

bool writeRawData( std::ofstream & file, char * buf, int len )
{
    // write the frame
    file.write( (char*)buf, len );

    // update the size
    mdatSize += len;

    return true;
}


bool updateAllSizeAndDuration( std::ofstream & file, int frame_rate, int frame_count )
{
    // update the duration in the MVHD atom
    float dur_secs = (float)frame_count / (float)frame_rate;
    unsigned int dur_ticks = (unsigned int)(dur_secs * (float)timeScale);

    std::cout << "Duration: " << dur_secs << " secs, " << dur_ticks << ", frame count = " << frame_count << std::endl;
    std::cout << "Total Ticks: " << dur_ticks << " (" << timeScale << ")" << std::endl;

    videoDuration = swapEndian(dur_ticks);
    file.seekp( mvhdDurationOffset );
    file.write( (char*)&videoDuration, 4 );

    // update the duration in the MDHD atom
    file.seekp( mdhdDurationOffset );
    file.write( (char*)&videoDuration, 4 );

    // update the duration in the TKHD atom
    file.seekp( tkhdDurationOffset );
    file.write( (char*)&videoDuration, 4 );

    // update the size in the MDAT atom
    file.seekp( mdatSizeOffset );
    unsigned int tmp = swapEndian(mdatSize);
    file.write( (char*)&tmp, 4 );

    return true;
}
