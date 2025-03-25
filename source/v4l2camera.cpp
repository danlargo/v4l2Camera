#include <cstring>
#include <string>
#include <iostream>
#include <vector>

#include "v4l2camera.h"

V4l2Camera::V4l2Camera()
{
    // default logging, keep internal buffer, 500 entries deep
    m_logMode = v4l2cam_logging_mode::logInternal;
    clearLog();

    // initiallize the return buffer
    m_frameBuffer = nullptr;

    // set the defaults
    m_userName = "unknown";
    m_modes.clear();
    m_controls.clear();
    m_cameraType = "unknown";
    m_capabilities = 0;
    m_metamode = -1;
    m_metasize = -1;

    // force to unhealthy state
    m_healthCounter = s_healthCountLimit;
}

V4l2Camera::~V4l2Camera()
{
}


std::string V4l2Camera::getCameraType()
{
    return m_cameraType;
}


std::string V4l2Camera::getDevName()
{
    return "";
}


std::string V4l2Camera::getUserName()
{
    return m_userName;
}

bool V4l2Camera::canFetch()
{
    return false;    
}

bool V4l2Camera::canRead()
{
    return false;
}

bool V4l2Camera::hasMetaData()
{
    return false;
}


void V4l2Camera::setLogMode( enum v4l2cam_logging_mode newMode )
{
    m_logMode = newMode;
}


void V4l2Camera::log( std::string out, enum v4l2cam_msg_type tag )
{
    if( v4l2cam_logging_mode::logOff != m_logMode )
    {
        // build the log message
        std::string msg = "[" + this->getTagStr(tag) + "] " + this->getDevName() +  " : " + out;

        // immediately add to the end of the list
        m_debugLog.push_back( msg );

        // check if we should do anything else with it
        if( v4l2cam_logging_mode::logToStdErr == m_logMode ) std::cerr << msg << std::endl;
        if( v4l2cam_logging_mode::logToStdOut == m_logMode ) std::cout << msg << std::endl;

        // if it is critical, always display it
        if( v4l2cam_msg_type::critical == tag ) std::cerr << msg << std::endl;

        // check the length of the log, if greater than logDepth then clean from beginning
        while( m_debugLog.size() > s_logDepth ) m_debugLog.erase( m_debugLog.begin() );

    } else m_debugLog.clear();       // make sure log is off and empty
}


std::vector<std::string> V4l2Camera::getLogMsgs( int count )
{
    std::vector<std::string> ret;

    // grab count messages off the top off the stack
    for( int i=0;i<count;i++ )
    {
        if( m_debugLog.size() > 0 )
        {
            ret.push_back( m_debugLog[0] );
            m_debugLog.erase( m_debugLog.begin() );        
        }
    }

    return ret;
}


std::string V4l2Camera::getTagStr( enum v4l2cam_msg_type tag )
{
    // \x1b[31m - Red
    // \x1b[32m - Green
    // \x1b[33m - Yellow/Orange
    // \x1b[0m - Normal

    switch( tag )
    {
        case v4l2cam_msg_type::info:
            return "\x1b[1;34mINFO\x1b[0m";
            break;
        case v4l2cam_msg_type::warning:
            return "\x1b[33mWARN\x1b[0m";
            break;
        case v4l2cam_msg_type::error:
            return "\x1b[1;31mERR \x1b[0m";
            break;
        case v4l2cam_msg_type::critical:
            return "\x1b[0;31mCRIT\x1b[0m";
            break;
        default:
            return "????";
            break;
    }
}


void V4l2Camera::clearLog()
{
    m_debugLog.clear();
}


int V4l2Camera::setValue( int id, int newVal, bool openOnDemand )
{
    return -1;
}


int V4l2Camera::getValue( int id, bool openOnDemand )
{
    return -1;
}


bool V4l2Camera::open()
{
    return false;
}


bool V4l2Camera::isOpen()
{
    return false;
}

bool V4l2Camera::isHealthy()
{
    // general operation, intention anyway
    //
    // incremenet m_healthCounter if a call fails, set to zero if call is successful
    // - allows for a coiuple of failed calls before declaring the camera unhealthy
    //
    // - in base class m_healthCounter is set to s_healthCountLimit in constructor, base class is always UN healthy

    // default subclass implementation
    return (m_healthCounter < s_healthCountLimit);
}


bool V4l2Camera::enumCapabilities()
{
    m_capabilities = 0;

    return false;
}

std::vector<std::string> V4l2Camera::capabilitiesToStr()
{
    std::vector<std::string> ret = {};

    return ret;
}



bool V4l2Camera::checkCapabilities( unsigned int val )
{
    return( m_capabilities & val );
}


void V4l2Camera::close()
{
}


struct v4l2cam_video_mode * V4l2Camera::getFrameFormat()
{
    return nullptr;
}


int V4l2Camera::getFrameRate()
{
    return -1;
}

bool V4l2Camera::setFrameRate( int fps )
{
    return false;
}


bool V4l2Camera::setFrameFormat( struct v4l2cam_video_mode vm, int fps )
{
    return false;
}


bool V4l2Camera::setFrameFormat( std::string mode, int width, int height, int fps )
{
    // lets see if we can find the requested mode
    for( auto x : m_modes )
    {
        if( (x.format_str == mode) && (x.width == width) && (x.height == height) )
        {
            return setFrameFormat( x, fps );
        }
    }

    log( "Requested mode not found", v4l2cam_msg_type::error);

    return false;
}


bool V4l2Camera::init( enum v4l2cam_fetch_mode newMode )
{
    return false;
}


struct v4l2cam_image_buffer * V4l2Camera::fetch( bool lastOne )
{
    struct v4l2cam_image_buffer * retBuffer = nullptr;

    return retBuffer;
}


struct v4l2cam_metadata_buffer * V4l2Camera::fetchMetaData()
{
    struct v4l2cam_metadata_buffer * retBuffer = nullptr;

    return retBuffer;
}



struct v4l2cam_video_mode V4l2Camera::getOneVM( int index )
{
    // check if it exists
    if( index >= m_modes.size() ) throw std::runtime_error("Video Mode does not exist");
    else return m_modes[index];
}

bool V4l2Camera::enumVideoModes()
{
    // clear the existing control structure
    m_modes.clear();

    return false;
}

bool V4l2Camera::enumMetadataModes()
{
    return false;
}

struct v4l2cam_control V4l2Camera::getOneCntrl( int index )
{
    // check if it exists
    if( m_controls.find(index) == this->m_controls.end() ) throw std::runtime_error("Control does not exist");
    else return m_controls[index];
}

bool V4l2Camera::enumControls()
{
    // clear the existing control structure
    m_controls.clear();

    return false;
}

std::string V4l2Camera::cntrlTypeToString( int type )
{
    std::string ret = "unknown";

    return ret;
}

// FourCC conversion helper methods
//
void V4l2Camera::fourcc_int_to_charArray( unsigned int fourcc, char * ret )
{
    ret[0] = (char)(fourcc & 0xFF);
    ret[1] = (char)((fourcc >> 8) & 0xFF);
    ret[2] = (char)((fourcc >> 16) & 0xFF);
    ret[3] = (char)((fourcc >> 24) & 0xFF);
    ret[4] = '\0';

}


unsigned int V4l2Camera::fourcc_charArray_to_int( unsigned char * fourcc )
{
    return (unsigned int)fourcc[3]<<24 | (unsigned int)fourcc[2]<<16 | (unsigned int)fourcc[1]<<8 | (unsigned int)fourcc[0];
}


unsigned int V4l2Camera::fourcc_intArray_to_int( int * fourcc )
{
    return fourcc[3]<<24 | fourcc[2]<<16 | fourcc[1]<<8 | fourcc[0];
}


std::string V4l2Camera::fourcc_int_to_descriptor( unsigned int fourcc )
{
    char in[5];
    V4l2Camera::fourcc_int_to_charArray(fourcc, in);
    std::string cmp = in;

    std::string ret = "unknown format [" + cmp + "]";

    if( cmp == "MJPG" ) return "Motion-JPEG [MJPG]       ";
    if( cmp == "H264" ) return "H.264 Video [H264]       ";
    if( cmp == "H265" ) return "H.265 Video [H265]       ";
    if( cmp == "YUYV" ) return "YUYV 4:2:2 [YUYV]        ";
    if( cmp == "YVYU" ) return "YVYU 4:2:2 [YVYU]        ";
    if (cmp == "YUY2" ) return "YUYV 4:2:2 [YUY2]        ";
    if( cmp == "YU12" ) return "Planar YUV 4:2:0 [YU12]  ";
    if( cmp == "I420" ) return "Planar YUV 4:2:0 [I420]  ";
    if( cmp == "NV12" ) return "Inter Y/UV 4:2:0 [NV12   ";
    if( cmp == "NV21" ) return "Y/UV 4:2:0 [NV21]        ";
    if( cmp == "Y16 " ) return "16-bit Greyscale [Y16 ]  ";
    if( cmp == "Y8  " ) return "8-bit Greyscale [Y8  ]   ";
    if( cmp == "Y800" ) return "8-bit Greyscale [Y800]   ";
    if( cmp == "GREY" ) return "8-bit Greyscale [GREY]   ";
    if( cmp == "420v" ) return "AVF High Resolution Video";
    if( cmp == "yuvs" ) return "AVF YUYV 4:2:2           ";
    if( cmp == "2vuy" ) return "AVF UYVY 4:2:2           ";

    return ret;
}
