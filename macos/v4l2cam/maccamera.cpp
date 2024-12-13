#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

#include <unistd.h>
#include <fcntl.h>

#include "maccamera.h"

MACCamera::MACCamera()
    : V4l2Camera()
{
    throw std::runtime_error("MACCamera::MACCamera() : Not currently supported, coming soon - using AVFoundation api");

}


MACCamera::~MACCamera()
{
}


void MACCamera::initAPI()
{
}


void MACCamera::closeAPI()
{
}


std::vector< MACCamera *> MACCamera::discoverCameras()
{
    std::vector< MACCamera *> camList;
    
    return camList;
}


std::string MACCamera::getDevName()
{
    return "-not set-";
}


bool MACCamera::canFetch()
{
    return false;    
}


bool MACCamera::canRead()
{
    return false;
}


int MACCamera::setValue( int id, int newVal, bool openOnDemand )
{
    int ret = -1;

    return ret;
}

int MACCamera::getValue( int id, bool openOnDemand )
{
    int ret = -1;

    return ret;
}

bool MACCamera::open()
{
    bool ret = false;
    
    return ret;
}


bool MACCamera::isOpen()
{
    return false;
}


bool MACCamera::enumCapabilities()
{
    bool ret = false;

    return ret;
}


void MACCamera::close()
{
}


bool MACCamera::setFrameFormat( struct v4l2cam_video_mode vm )
{
    return false;
}


bool MACCamera::init( enum v4l2cam_fetch_mode newMode )
{
    bool ret = false;

    if( !this->isOpen() ) log( "Unable to call init() as device is NOT open", info );

    else 
    {
        ret = false;
    }

    return ret;
}

struct v4l2cam_image_buffer * MACCamera::fetch( bool lastOne )
{
    struct v4l2cam_image_buffer * retBuffer = nullptr;

    if( !this->isOpen() ) log( "Unable to call fetch() as no device is open", warning );

    else 
    {
    }

    return retBuffer;
}


bool MACCamera::enumVideoModes()
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


bool MACCamera::enumControls()
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

std::string MACCamera::cntrlTypeToString( int type )
{
    std::string ret = "unknown";

    switch( type )
    {
        default:
            break;
    }

    return ret;
}
