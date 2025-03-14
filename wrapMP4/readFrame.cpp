#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <map>
#include <string>

#include "wrapMP4.h"

struct h264FrameHeader_t * getFrameHeader( std::ifstream & inFile )
{
    struct h264FrameHeader_t * header = new h264FrameHeader_t;

    // read a parameter at a time
    inFile.read( (char*)&header->delimiter, 4 );

    // make sure delimiter is correct
    if( (header->delimiter[0] != 's') || (header->delimiter[1] != 'l') || (header->delimiter[2] != 'a') || (header->delimiter[3] != 'p') )
    {
        delete header;
        return NULL;
    }

    inFile.read( (char*)&header->rate, 4 );
    header->rate = swapEndian( header->rate );
    inFile.read( (char*)&header->width, 4 );
    header->width = swapEndian( header->width );
    inFile.read( (char*)&header->height, 4 );
    header->height = swapEndian( header->height );
    inFile.read( (char*)&header->h264_frame_type, 4 );
    header->h264_frame_type = swapEndian( header->h264_frame_type );
    inFile.read( (char*)&header->sps_offset, 4 );
    header->sps_offset = swapEndian( header->sps_offset );
    inFile.read( (char*)&header->pps_offset, 4 );
    header->pps_offset = swapEndian( header->pps_offset );
    inFile.read( (char*)&header->frame_offset, 4 );
    header->frame_offset = swapEndian( header->frame_offset );
    inFile.read( (char*)&header->frame_size, 4 );
    header->frame_size = swapEndian( header->frame_size );

    return header;

}
