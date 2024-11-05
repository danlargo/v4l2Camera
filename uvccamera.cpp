#include <cstring>
#include <string>
#include <iostream>
#include <vector>

#include "uvccamera.h"

UVCCamera::UVCCamera()
{
    // default logging, keep internal buffer, 500 entries deep
    this->m_logMode = logInternal;
    clearLog();

    // initiallize the return buffer
    m_frameBuffer = nullptr;

    // set the defaults
    m_userName = "unknown";
}

UVCCamera::~UVCCamera()
{
}


std::string UVCCamera::getCameraType()
{
    return "uvc camera";
}

std::string UVCCamera::getDevName()
{
    return "";
}

std::string UVCCamera::getUserName()
{
    return m_userName;
}

bool UVCCamera::canOpen()
{
    return false;
}

bool UVCCamera::canFetch()
{
    return false;    
}

bool UVCCamera::canRead()
{
    return false;
}

void UVCCamera::setLogMode( enum logging_mode newMode )
{
    this->m_logMode = newMode;
}

void UVCCamera::log( std::string out, enum msg_type tag )
{
    if( logOff != this->m_logMode )
    {
        // build the log message
        std::string msg = "[" + this->getTagStr(tag) + "] " + this->getDevName() +  " : " + out;

        // immediately add to the end of the list
        this->m_debugLog.push_back( msg );

        // check if we should do anything else with it
        if( logToStdErr == this->m_logMode ) std::cerr << msg << std::endl;
        if( logToStdOut == this->m_logMode ) std::cout << msg << std::endl;

        // check the length of the log, if greater than logDepth then clean from beginning
        while( this->m_debugLog.size() > this->s_logDepth ) this->m_debugLog.erase(this->m_debugLog.begin() );

    } else this->m_debugLog.clear();       // make sure log is off and empty
}

std::string UVCCamera::getTagStr( enum msg_type tag )
{
    // \x1b[31m - Red
    // \x1b[32m - Green
    // \x1b[33m - Yellow/Orange
    // \x1b[0m - Normal

    switch( tag )
    {
        case info:
            return "\x1b[1;34mINFO\x1b[0m";
            break;
        case warning:
            return "\x1b[33mWARN\x1b[0m";
            break;
        case error:
            return "\x1b[1;31mERR \x1b[0m";
            break;
        case critical:
            return "\x1b[0;31mCRIT\x1b[0m";
            break;
        default:
            return "????";
            break;
    }
}

void UVCCamera::clearLog()
{
    this->m_debugLog.clear();
}

int UVCCamera::setValue( int id, int newVal, bool openOnDemand )
{
    return -1;
}

int UVCCamera::getValue( int id, bool openOnDemand )
{
    return -1;
}

bool UVCCamera::open()
{
    return false;
}

bool UVCCamera::isOpen()
{
    return false;
}

bool UVCCamera::enumCapabilities()
{
    m_capabilities = 0;

    return false;
}

bool UVCCamera::checkCapabilities( unsigned int val )
{
    return (this->m_capabilities & val);
}

void UVCCamera::close()
{
}

bool UVCCamera::setFrameFormat( struct video_mode vm )
{
    return false;
}

bool UVCCamera::setFrameFormat( std::string mode, int width, int height )
{
    // lets see if we can find the requested mode
    for( auto x : this->m_modes )
    {
        struct video_mode tmpMode = x.second;
        if( (tmpMode.format_str == mode) && (tmpMode.width == width) && (tmpMode.height == height) )
        {
            return setFrameFormat( tmpMode );
        }
    }

    log( "Requested mode not found");

    return false;
}

bool UVCCamera::init( enum fetch_mode newMode )
{
    return false;
}

struct image_buffer * UVCCamera::fetch( bool lastOne )
{
    struct image_buffer * retBuffer = nullptr;

    return retBuffer;
}

struct video_mode UVCCamera::getOneVM( int index )
{
    // check if it exists
    if( this->m_modes.find(index) == this->m_modes.end() ) throw std::runtime_error("Video Mode does not exist");
    else return this->m_modes[index];
}

bool UVCCamera::enumVideoModes()
{
    // clear the existing control structure
    this->m_modes.clear();

    return false;
}

struct user_control UVCCamera::getOneCntrl( int index )
{
    // check if it exists
    if( this->m_controls.find(index) == this->m_controls.end() ) throw std::runtime_error("Control does not exist");
    else return this->m_controls[index];
}

bool UVCCamera::enumControls()
{
    // clear the existing control structure
    this->m_controls.clear();

    return false;
}

std::string UVCCamera::cntrlTypeToString( int type )
{
    std::string ret = "unknown";

    return ret;
}
