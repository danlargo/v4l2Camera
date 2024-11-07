#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

#include "libuvc.h"

#include <unistd.h>
#include <fcntl.h>

#include "maccamera.h"
#include "v4l2_defs.h"

MACCamera::MACCamera( struct uvc_device * dev )
    : V4l2Camera()
{
    // save the device information
    m_dev = dev;

    m_userName = dev->productName;
    m_Handle = nullptr;

}


MACCamera::~MACCamera()
{
    if( m_Handle ) uvc_close( m_Handle );
}


void MACCamera::initAPI()
{
    uvc_init( &UVC_ctx, nullptr );
}


void MACCamera::closeAPI()
{
    uvc_exit( UVC_ctx );
}


std::vector< MACCamera *> MACCamera::discoverCameras()
{
    std::vector< MACCamera *> camList;
    int count = 0;

    std::vector<struct uvc_device *> devList = MACCamera::buildCamList();

    for( const auto &x : devList )
    {
        bool keep = false;

        // create the camera object
        MACCamera * tmpC = new MACCamera(x);
        tmpC->setLogMode( logging_mode::logToStdErr );

        std::string nam = x->productName;

        // check if this is actually a UVC camera
        if( tmpC )
        {
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

                        } else tmpC->log( nam + " : zero video modes detected", info );
                    } else tmpC->log( nam + " : does not support video capture", info  );
                } else tmpC->log( nam + " : unable to query capabilities", info  );

                // close the camera
                tmpC->close();

            } else tmpC->log( nam + " : failed to open device", info  );
        } else tmpC->log( nam + " : failed to create V4l2Camera object for device", info );

        if( !keep ) delete tmpC;
    }

    return camList;
}


std::vector<struct uvc_device *> MACCamera::buildCamList()
{
    std::vector<struct uvc_device *> ret;

    // grab a simple list of USB devices to start our libusb journey
    uvc_device **devs;

    //if( uvc_find_devices( UVC_ctx, &devs, 0, 0, nullptr ) == UVC_SUCCESS )
    if( uvc_get_device_list( UVC_ctx, &devs ) == UVC_SUCCESS )
    {
        uvc_device *dev;
        int i = 0;

        while ((dev = devs[i++]) != NULL) 
        {
            struct uvc_device * tmp = new struct uvc_device;
            struct uvc_device_descriptor * desc;
            if( uvc_get_device_descriptor(dev, &desc) == UVC_SUCCESS )
            {
                tmp->bus = uvc_get_bus_number(dev);
                tmp->address = uvc_get_device_address(dev);
                tmp->vid = desc->idVendor;
                tmp->pid = desc->idProduct;
                if( desc->manufacturer ) tmp->manufacturerName = desc->manufacturer;
                if( desc->product ) tmp->productName = desc->product;
                if( desc->serialNumber ) tmp->serialNumber = desc->serialNumber;

                // save this device information
                ret.push_back( tmp );

                uvc_free_device_descriptor(desc);
            
            } else std:: cerr << "Failed to get device descriptor" << std::endl;
        }

        uvc_free_device_list(devs, 1);
    }

    return ret;
}


std::string MACCamera::getCameraType()
{
    return "macos usb camera";
}


std::string MACCamera::getDevName()
{
    // Example : Bus 003 Device 005: ID 289d:0011 Seek Thermal, Inc. PIR324 Thermal Camera
    std::stringstream ss;

    if( m_dev )
    {
        ss << "Bus " << std::dec << std::setw(3) << std::setfill('0') << m_dev->bus;
        ss << " Device " << std::dec << std::setw(3) << std::setfill('0') << m_dev->address;
        ss << ": ID " << std::hex << std::setw(4) << std::setfill('0') << m_dev->vid;
        ss << ":" << std::hex << std::setw(4) << std::setfill('0') << m_dev->pid;
        const std::string s = ss.str();
        return s;

    } else return "USB Device, address not set";
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

    // libusb stuff goes here eventually 

    return ret;
}

int MACCamera::getValue( int id, bool openOnDemand )
{
    int ret = -1;

    // libusb stuff goes here eventually 

    return ret;
}

bool MACCamera::open()
{
    bool ret = false;

    // find the device
    struct uvc_device * dev = nullptr;
    if( uvc_find_device( UVC_ctx, &dev, m_dev->vid, m_dev->pid, nullptr ) == UVC_SUCCESS )
    {
        // open the device
        uvc_error_t err = uvc_open( dev, &m_Handle);

        if( UVC_SUCCESS == err ) ret = true;

        else 
        {
            log( "Failed to open UVC camera : " + std::string(uvc_strerror(err)),  msg_type::critical );
            m_Handle = nullptr;

            // check for special case, access denied on MACOS...console apps CAN NOT request camera persmissions, they must be run as ROOT
            // sudo ./v4l2cam -l            ... if this lists cameras then you are good to go
            //
            #ifdef __APPLE__
                // this is likely a permisssions issue
                if(UVC_ERROR_ACCESS == err ) log( "You are running on MACOS, console apps CAN NOT request camera permissions, run as root", msg_type::critical ); 
            #endif
        }
    }

    return ret;
}


bool MACCamera::isOpen()
{
    return( m_Handle != nullptr );
}


bool MACCamera::enumCapabilities()
{
    bool ret = false;

    if( !this->isOpen() ) log( "Unable to call getCaps() as device is NOT open", warning );

    else
    {
        // grab the device name
        this->m_userName = m_dev->manufacturerName + " " + m_dev->productName;

        // libusb stuff goes here eventually 
        m_capabilities = 0;

        ret = true;
    }

    return ret;
}


void MACCamera::close()
{
    if( m_Handle != nullptr ) uvc_close( m_Handle );
    m_Handle = nullptr;
}


bool MACCamera::setFrameFormat( struct video_mode vm )
{
    bool ret = false;

    // libusb stuff goes here eventually

    return ret;
}


bool MACCamera::init( enum fetch_mode newMode )
{
    bool ret = false;

    if( !this->isOpen() ) log( "Unable to call init() as device is NOT open", info );

    else 
    {
        // libusb stuff goes here eventually

        ret = false;

    }

    return ret;
}

struct image_buffer * MACCamera::fetch( bool lastOne )
{
    struct image_buffer * retBuffer = nullptr;

    if( !this->isOpen() ) log( "Unable to call fetch() as no device is open", warning );

    else 
    {
        // libusb stuff goes here eventually

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
        // get the list of format descriptors
        const uvc_format_desc_t *format_desc = uvc_get_format_descs( m_Handle );

        // walk the list of pixel formats, for each format, walk the list of video modes (sizes), put into m_modes structure
        while( format_desc != nullptr )
        {
            const uvc_frame_desc_t *frame_desc = format_desc->frame_descs;

            while( frame_desc != nullptr )
            {
                struct video_mode tmpMode;

                tmpMode.fourcc = fourcc_charArray_to_int( (unsigned char*)format_desc->fourccFormat );
                tmpMode.format_str = fourcc_int_to_descriptor( tmpMode.fourcc );
                tmpMode.width = frame_desc->wWidth;
                tmpMode.height = frame_desc->wHeight;
                tmpMode.size = frame_desc->dwBytesPerLine * frame_desc->wHeight;

                // save all the info
                this->m_modes.push_back( tmpMode );

                frame_desc = frame_desc->next;
            }
            format_desc = format_desc->next;
        }
    }

    return ret;
}


bool MACCamera::enumControls()
{
    bool ret = false;

    // clear the existing control structure
    this->m_controls.clear();

    // make sure fid is valid
    if( !this->isOpen() ) log( "Unable to call enumControls() as device is NOT open", warning );

    else
    {

        // libusb stuff goes here eventually

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
