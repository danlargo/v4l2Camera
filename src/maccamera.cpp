#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

#include <unistd.h>
#include <fcntl.h>

#include "maccamera.h"

MACCamera::MACCamera( std::string device_name )
    : V4l2Camera()
{
    m_devName = device_name;
    m_userName = "";
    m_capabilities = 0;
    m_fid = -1;
    m_bufferMode = notset;

    m_cameraType = "AVFoundation camera";

    m_healthCounter = 0;

    m_objcCamera = new i_ObjCCamera();
    if( m_objcCamera == nullptr ) throw std::runtime_error( "Unable to create ObjC Camera object" );

}


MACCamera::~MACCamera()
{
    if( m_objcCamera ) delete m_objcCamera;
}


std::vector< MACCamera *> MACCamera::discoverCameras()
{
    std::vector< MACCamera *> camList;
    bool keep = false;

    // invoke the ObjC method to discover cameras
    std::vector<struct avCamera*> tmpList;

    // create a temporary ObjC camera object to access camera list
    i_ObjCCamera * tmpI = new i_ObjCCamera();
    if( tmpI )
    {
        tmpList = tmpI->discoverCameras();
        delete tmpI;
    }

    // map to the C++ camera object
    for( int i=0;i<tmpList.size(); i++ )
    {
        struct avCamera * c = tmpList[i];
        MACCamera * tmpC = new MACCamera( c->uniqueID );
        if( tmpC )
        {
            tmpC->m_userName = c->name + " : " + c->manufacturer;
            tmpC->m_devName = c->uniqueID;
            tmpC->m_manufacturer = c->manufacturer;
            tmpC->m_modelID = c->modelID;
            tmpC->m_deviceType = c->deviceType;
            tmpC->m_rearFacing = c->rearFacing;

            // open the camera so we can query all its capabilities
            if( tmpC->open() )
            {
                if( tmpC->enumCapabilities() )
                {
                    if( tmpC->canFetch() )
                    {
                        // have it query its own capabilities
                        tmpC->enumControls();
                        tmpC->enumVideoModes();

                        if( tmpC->getVideoModes().size() > 0 )
                        {
                            // save the camera for display later
                            keep = true;
                            camList.push_back(tmpC);

                        } else tmpC->log( tmpC->m_userName + " : zero video modes detected", info );
                    } else tmpC->log( tmpC->m_userName + " : does not support video capture", info  );
                } else tmpC->log( tmpC->m_userName + " : unable to query capabilities", info  );

                // close the camera
                tmpC->close();

            } else tmpC->log( tmpC->m_userName + " : failed to open device", info  );
        }

        if( !keep ) delete tmpC;
    }

    return camList;
}


std::string MACCamera::getDevName()
{
    return m_devName;
}


bool MACCamera::canFetch()
{
    return true;    
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
    
    if( m_objcCamera ) ret = m_objcCamera->open( m_devName );

    return ret;
}


bool MACCamera::isOpen()
{
    return (m_objcCamera != nullptr );
}


bool MACCamera::enumCapabilities()
{
    bool ret = true;

    return ret;
}


void MACCamera::close()
{
    if( m_objcCamera ) m_objcCamera->close();
}


bool MACCamera::setFrameFormat( struct v4l2cam_video_mode vm )
{
    // find the offset into the video modes, and use that offset to request mode
    for( int i=0;i<m_modes.size(); i++ )
    {
        if( (m_modes[i].width == vm.width) && 
            (m_modes[i].height == vm.height) &&
            (m_modes[i].fourcc == vm.fourcc) )
        {
            // set the mode
            return m_objcCamera->setCaptureFormat( i );
        }
    }
    return false;
}


bool MACCamera::init( enum v4l2cam_fetch_mode newMode )
{
    bool ret = false;

    if( !this->isOpen() ) log( "Unable to call init() as device is NOT open", info );

    else ret = m_objcCamera->initCapture();

    return ret;
}

struct v4l2cam_image_buffer * MACCamera::fetch( bool lastOne )
{
    struct v4l2cam_image_buffer * retBuffer = nullptr;
    int size;

    if( !this->isOpen() ) log( "Unable to call fetch() as no device is open", warning );

    else retBuffer->buffer = m_objcCamera->captureFrame( &size );

    return retBuffer;
}


bool MACCamera::enumVideoModes()
{
    bool ret = false;

    // clear the existing video structure
    this->m_modes.clear();

    // make sure fid is valid
    if( !this->isOpen() )  log( "Unable to call enumVideoModes() as device is NOT open", warning );
    else  
    {
        std::vector<struct avFormat*> tmpList = m_objcCamera->getVideoFormats();

        // convert the ObjC video formats to the C++ video modes
        for( int i=0;i<tmpList.size(); i++ )
        {
            struct avFormat * f = tmpList[i];
            struct v4l2cam_video_mode tmpVM;

            tmpVM.fourcc = f->fourCC;
            tmpVM.format_str = fourcc_int_to_descriptor( f->fourCC );
            tmpVM.size = f->width * 4 * f->height;
            tmpVM.width = f->width;
            tmpVM.height = f->height;

            m_modes.push_back(tmpVM);
        }

        ret = true;
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
