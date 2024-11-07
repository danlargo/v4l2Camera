#include <cstring>
#include <string>
#include <iostream>
#include <vector>

#include "v4l2camera.h"
#include "v4l2_defs.h"

V4l2Camera::V4l2Camera()
{
    // default logging, keep internal buffer, 500 entries deep
    m_logMode = logging_mode::logInternal;
    clearLog();

    // initiallize the return buffer
    m_frameBuffer = nullptr;

    // set the defaults
    m_userName = "unknown";
    m_modes.clear();
    m_controls.clear();
}

V4l2Camera::~V4l2Camera()
{
}


std::string V4l2Camera::getCameraType()
{
    return "v4l2 camera";
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


void V4l2Camera::setLogMode( enum logging_mode newMode )
{
    m_logMode = newMode;
}


void V4l2Camera::log( std::string out, enum msg_type tag )
{
    if( logging_mode::logOff != m_logMode )
    {
        // build the log message
        std::string msg = "[" + this->getTagStr(tag) + "] " + this->getDevName() +  " : " + out;

        // immediately add to the end of the list
        m_debugLog.push_back( msg );

        // check if we should do anything else with it
        if( logging_mode::logToStdErr == m_logMode ) std::cerr << msg << std::endl;
        if( logging_mode::logToStdOut == m_logMode ) std::cout << msg << std::endl;

        // check the length of the log, if greater than logDepth then clean from beginning
        while( m_debugLog.size() > s_logDepth ) m_debugLog.erase( m_debugLog.begin() );

    } else m_debugLog.clear();       // make sure log is off and empty
}


std::string V4l2Camera::getTagStr( enum msg_type tag )
{
    // \x1b[31m - Red
    // \x1b[32m - Green
    // \x1b[33m - Yellow/Orange
    // \x1b[0m - Normal

    switch( tag )
    {
        case msg_type::info:
            return "\x1b[1;34mINFO\x1b[0m";
            break;
        case msg_type::warning:
            return "\x1b[33mWARN\x1b[0m";
            break;
        case msg_type::error:
            return "\x1b[1;31mERR \x1b[0m";
            break;
        case msg_type::critical:
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


bool V4l2Camera::enumCapabilities()
{
    m_capabilities = 0;

    return false;
}


bool V4l2Camera::checkCapabilities( unsigned int val )
{
    return( m_capabilities & val );
}


void V4l2Camera::close()
{
}


bool V4l2Camera::setFrameFormat( struct video_mode vm )
{
    return false;
}


bool V4l2Camera::setFrameFormat( std::string mode, int width, int height )
{
    // lets see if we can find the requested mode
    for( auto x : m_modes )
    {
        if( (x.format_str == mode) && (x.width == width) && (x.height == height) )
        {
            return setFrameFormat( x );
        }
    }

    log( "Requested mode not found", msg_type::error);

    return false;
}


bool V4l2Camera::init( enum fetch_mode newMode )
{
    return false;
}


struct image_buffer * V4l2Camera::fetch( bool lastOne )
{
    struct image_buffer * retBuffer = nullptr;

    return retBuffer;
}


struct video_mode V4l2Camera::getOneVM( int index )
{
    // check if it exists
    if( index >= m_modes.size() ) throw std::runtime_error("Video Mode does not exist");
    else return m_modes[index];
}

bool V4l2Camera::enumVideoModes()
{
    // clear the existing control structure
    this->m_modes.clear();

    return false;
}

struct user_control V4l2Camera::getOneCntrl( int index )
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
