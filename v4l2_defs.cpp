#include "v4l2_defs.h"

void fourcc_int_to_charArray( unsigned int fourcc, char * ret )
{
    ret[0] = (char)(fourcc & 0xFF);
    ret[1] = (char)((fourcc >> 8) & 0xFF);
    ret[2] = (char)((fourcc >> 16) & 0xFF);
    ret[3] = (char)((fourcc >> 24) & 0xFF);
    ret[4] = '\0';
}


unsigned int fourcc_charArray_to_int( unsigned char * fourcc )
{
    return (unsigned int)fourcc[3]<<24 | (unsigned int)fourcc[2]<<16 | (unsigned int)fourcc[1]<<8 | (unsigned int)fourcc[0];
}


unsigned int fourcc_intArray_to_int( int * fourcc )
{
    return fourcc[3]<<24 | fourcc[2]<<16 | fourcc[1]<<8 | fourcc[0];
}


std::string fourcc_int_to_descriptor( unsigned int fourcc )
{
    char in[5];
    fourcc_int_to_charArray(fourcc, in);
    std::string cmp = in;

    std::string ret = "unknown format [" + cmp + "]";

    if( cmp == "MJPG" ) return "Motion-JPEG [MJPG]";
    if( cmp == "H264" ) return "H.264 Video [H264]";
    if( cmp == "H265" ) return "H.265 Video [H265]";
    if( cmp == "YUYV" ) return "YUYV 4:2:2 [YUYV]";
    if( cmp == "YVYU" ) return "YVYU 4:2:2 [YVYU]";
    if (cmp == "YUY2" ) return "YUYV 4:2:2 [YUY2]";
    if( cmp == "YU12" ) return "Planar YUV 4:2:0 [YU12]";
    if( cmp == "I420" ) return "Planar YUV 4:2:0 [I420]";
    if( cmp == "NV12" ) return "Interleaved Y/UV 4:2:0 [NV12]";
    if( cmp == "NV21" ) return "Y/UV 4:2:0 [NV21]";
    if( cmp == "Y16 " ) return "16-bit Greyscale [Y16 ]";
    if( cmp == "Y8  " ) return "8-bit Greyscale [Y8  ]";
    if( cmp == "Y800" ) return "8-bit Greyscale [Y800]";
    if( cmp == "GREY" ) return "8-bit Greyscale [GREY]";

    return ret;
}