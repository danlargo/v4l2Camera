#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

#include <unistd.h>
#include <fcntl.h>

#include "wincamera.h"

WinCamera::WinCamera()
    : V4l2Camera()
{
    throw std::runtime_error("MACCamera::MACCamera() : Not currently supported, coming soon - using AVFoundation api");

}


WinCamera::~WinCamera()
{
}


std::vector< WinCamera *> WinCamera::discoverCameras()
{
    std::vector< WinCamera *> camList;
    
    return camList;
}


std::string WinCamera::getDevName()
{
    return "-not set-";
}


bool WinCamera::canFetch()
{
    return false;    
}


bool WinCamera::canRead()
{
    return false;
}


int WinCamera::setValue( int id, int newVal, bool openOnDemand )
{
    int ret = -1;

    return ret;
}

int WinCamera::getValue( int id, bool openOnDemand )
{
    int ret = -1;

    return ret;
}

bool WinCamera::open()
{
    bool ret = false;
    
    return ret;
}


bool WinCamera::isOpen()
{
    return false;
}


bool WinCamera::enumCapabilities()
{
    bool ret = false;

    return ret;
}


void WinCamera::close()
{
}


bool WinCamera::setFrameFormat( struct v4l2cam_video_mode vm )
{
    return false;
}


bool WinCamera::init( enum v4l2cam_fetch_mode newMode )
{
    bool ret = false;

    if( !this->isOpen() ) log( "Unable to call init() as device is NOT open", info );

    else 
    {
        ret = false;
    }

    return ret;
}

struct v4l2cam_image_buffer * WinCamera::fetch( bool lastOne )
{
    struct v4l2cam_image_buffer * retBuffer = nullptr;

    if( !this->isOpen() ) log( "Unable to call fetch() as no device is open", warning );

    else 
    {
    }

    return retBuffer;
}


bool WinCamera::enumVideoModes()
{
    bool ret = false;

    // clear the existing video structure
    this->m_modes.clear();

    // make sure fid is valid
    if( !this->isOpen() ) log( "Unable to call enumVideoModes() as device is NOT open", warning );

    else
    {
    }
    return ret;
}


bool WinCamera::enumControls()
{
    bool ret = false;

    // clear the existing control structure
    m_controls.clear();

    // make sure fid is valid
    if( !isOpen() ) log( "Unable to call enumControls() as device is NOT open", warning );

    else
    {
    }

    return ret;
}

std::string WinCamera::cntrlTypeToString( int type )
{
    std::string ret = "unknown";

    switch( type )
    {
        default:
            break;
    }

    return ret;
}
