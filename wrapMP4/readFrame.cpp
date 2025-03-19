#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <map>
#include <string>
#include <cstring>

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

struct mp4StreamInfo_t * getStreamInfo( std::ifstream & inFile )
{
    struct h264FrameHeader_t * header = getFrameHeader( inFile );
    if( header == NULL ) return nullptr;

    struct mp4StreamInfo_t * info = new mp4StreamInfo_t;
    bool gotSPS = false;

    // initialize the MDAT data size
    info->mdatSize = 0;

    while( !inFile.eof() )
    {
        // grab the info from each frame, just in case is changes (rate will update as output engine calculates aversage fps)
        info->width = header->width;
        info->height = header->height;
        info->frame_rate = header->rate;

        // read the frame
        unsigned char * buf = new unsigned char[header->frame_size];
        inFile.read( (char*)buf, header->frame_size );

        // extract an SPS and PPS header for analysis, do this once
        if( !gotSPS )
        {
            // offset over the delimiter (00 00 00 01)
            unsigned int sps_offset = header->sps_offset+4;
            unsigned int pps_offset = header->pps_offset+4;
            
            if( pps_offset > 0 )
            {
                info->sps_size = header->pps_offset - sps_offset;
                info->pps_size = header->frame_offset - pps_offset;

                info->sps = new unsigned char[info->sps_size];
                memcpy( info->sps, buf + sps_offset, info->sps_size );
                info->pps = new unsigned char[info->pps_size];
                memcpy( info->pps, buf + pps_offset, info->pps_size );

                gotSPS = true;
            }
        }

        // figure out where h264 data frame starts, include 00 00 00 01 tag
        unsigned int data_start = header->frame_offset;
        unsigned int data_size = header->frame_size - data_start;

        // save the frame size, only data size, not the header(s)
        info->frame_sizes.push_back( data_size );
        info->frame_count++;
        info->mdatSize += data_size;

        delete[] buf;

        // get the next frame header
        delete header;
        header = getFrameHeader( inFile );
        if( header == NULL ) break;
    }

    return info;
}
