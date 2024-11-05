#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

#include <libusb-1.0/libusb.h>

#include <unistd.h>
#include <fcntl.h>

#include "maccamera.h"

MACCamera::MACCamera( struct usb_device * dev )
{
    // save the device information
    this->m_dev = new struct usb_device;

    m_dev->bus = dev->bus;
    m_dev->address = dev->address;
    m_dev->vid = dev->vid;
    m_dev->pid = dev->pid;
    m_dev->manufacturerName = dev->manufacturerName;
    m_dev->productName = dev->productName;

    m_userName = dev->productName;
    m_Handle = nullptr;

}

MACCamera::~MACCamera()
{
    if( m_Handle ) libusb_close(m_Handle);
}

void MACCamera::initAPI()
{
    libusb_init(NULL);
}

void MACCamera::closeAPI()
{
    libusb_exit(NULL);
}

std::map<int, MACCamera *> MACCamera::discoverCameras()
{
    std::map<int, MACCamera *> camList;
    int count = 0;

    std::vector<struct usb_device *> devList = MACCamera::buildCamList();

    for( const auto &x : devList )
    {
        bool keep = false;

        // create the camera object
        MACCamera * tmpC = new MACCamera(x);
        tmpC->setLogMode( logToStdOut );

        std::string nam = x->productName;

        // check if this is actually a UVC camera
        if( tmpC )
        {
            if( tmpC->canOpen() )
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
                                camList[count++] = tmpC;

                            } else tmpC->log( nam + " : zero video modes detected", info );
                        } else tmpC->log( nam + " : does not support video capture", info  );
                    } else tmpC->log( nam + " : unable to query capabilities", info  );

                    // close the camera
                    tmpC->close();

                } else tmpC->log( nam + " : failed to open device", info  );
            } else tmpC->log( nam + " : device indicates unable to open", info  );
        } else tmpC->log( nam + " : failed to create V4l2Camera object for device", info );

        if( !keep ) delete tmpC;
    }

    return camList;
}


std::vector<struct usb_device *> MACCamera::buildCamList()
{
    std::vector<struct usb_device *> ret;

    // grab a simple list of USB devices to start our libusb journey
    libusb_device **devs;

    ssize_t cnt = libusb_get_device_list(NULL, &devs);

    // now try to determine if they are UVC Cameras
    if (cnt >= 0)
    {
        libusb_device *dev;
        int i = 0, j = 0;
        uint8_t path[8];

        while ((dev = devs[i++]) != NULL) {
            struct libusb_device_descriptor desc;
            int r = libusb_get_device_descriptor(dev, &desc);
            if (r < 0)
            {
                std::cerr << "Failed to get device descriptor for device : " << r << std::endl;
                break;
            }
            int bus_num = libusb_get_bus_number(dev);
            int dev_addr = libusb_get_device_address(dev);

            libusb_device_handle *hHandle;
            int ret_val = libusb_open(dev, &hHandle);
            if( ret_val == LIBUSB_SUCCESS )
            {
                uint8_t got_interface = 0;
                struct libusb_config_descriptor *config;
                const struct libusb_interface *interface;
                const struct libusb_interface_descriptor *if_desc;

                libusb_get_config_descriptor(dev, 0, &config);

                for (int interface_idx = 0; !got_interface && interface_idx < config->bNumInterfaces; ++interface_idx)
                {
                    interface = &config->interface[interface_idx];

                    for (int altsetting_idx = 0; !got_interface && altsetting_idx < interface->num_altsetting; ++altsetting_idx)
                    {
                        if_desc = &interface->altsetting[altsetting_idx];

                        // Skip TIS cameras that definitely aren't UVC even though they might look that way
                        if ( 0x199e == desc.idVendor && desc.idProduct  >= 0x8201 && desc.idProduct <= 0x8208 ) continue;

                        // Special case for Imaging Source cameras
                        if( (0x199e == desc.idVendor) && 
                            ( (0x8101 == desc.idProduct) || (0x8102 == desc.idProduct) ) && 
                            (if_desc->bInterfaceClass == 255) && 
                            ( if_desc->bInterfaceSubClass == 2) ) got_interface = 1;

                        // UVC - Video, Streaming Device
                        if( (if_desc->bInterfaceClass == 14) && 
                            (if_desc->bInterfaceSubClass == 2) ) got_interface = 1;
                    }
                }

                libusb_free_config_descriptor(config);

                if( got_interface ) 
                {
                    unsigned char str_product[255] = {};
                    unsigned char str_manuf[255] = {};
                    unsigned char str_vendor[255] = {};
                    unsigned char str_serial[255] = {};

                    libusb_get_string_descriptor_ascii(hHandle, desc.iProduct, str_product, sizeof(str_product));
                    libusb_get_string_descriptor_ascii(hHandle, desc.iManufacturer, str_manuf, sizeof(str_manuf));
                    libusb_get_string_descriptor_ascii(hHandle, desc.iSerialNumber, str_serial, sizeof(str_serial));

                    // save the information
                    struct usb_device * newDev = new struct usb_device;
                    newDev->bus = bus_num;
                    newDev->address = dev_addr;
                    newDev->vid = desc.idVendor;
                    newDev->pid = desc.idProduct;
                    newDev->productName = (char *)str_product;
                    newDev->manufacturerName = (char *)str_manuf;
                    newDev->serialNumber = (char *)str_serial;
                    ret.push_back(newDev);

                }
                // close the device handler
                libusb_close(hHandle);
            }
        }
        libusb_free_device_list(devs, 1);
    }

    return ret;
}


std::string MACCamera::getCameraType()
{
    return "generic";
}

std::string MACCamera::getDevName()
{
    // Bus 003 Device 005: ID 289d:0011 Seek Thermal, Inc. PIR324 Thermal Camera

    std::stringstream ss;

    if( m_dev )
    {

        ss << "BUS  " << std::dec << std::setw(3) << std::setfill('0') << m_dev->bus;
        ss << " Device " << std::dec << std::setw(3) << std::setfill('0') << m_dev->address;
        ss << ": ID " << std::hex << std::setw(4) << std::setfill('0') << m_dev->vid;
        ss << ":" << std::hex << std::setw(4) << std::setfill('0') << m_dev->pid;
        const std::string s = ss.str();
        return s;

    } else return "USB Device, address not set";
}


bool MACCamera::canOpen()
{
    return true;
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

    libusb_device **devs;
    if( libusb_get_device_list(NULL, &devs) >= 0)
    {
        libusb_device *dev;
        int i = 0, j = 0;
        uint8_t path[8];

        // find the device we want to open
        while ((dev = devs[i++]) != NULL) 
        {
            struct libusb_device_descriptor desc;
            if( libusb_get_device_descriptor(dev, &desc) < 0) continue;

            int bus_num = libusb_get_bus_number(dev);
            int dev_addr = libusb_get_device_address(dev);

            if( (bus_num == m_dev->bus) && (dev_addr == m_dev->address) && (m_dev->vid == desc.idVendor) && (m_dev->pid == desc.idProduct) )
            {
                libusb_device_handle * hHandle;
                if( libusb_open(dev, &hHandle) == LIBUSB_SUCCESS ) this->m_Handle = hHandle;
                // this is the ONE, stop looking
                break;
            }
        }
        libusb_free_device_list(devs, 1);
    }

    if( m_Handle ) ret = true;

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
    if( m_Handle != nullptr ) libusb_close(m_Handle);
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
    int offset = 0;
    bool ret = false;

    // clear the existing video structure
    this->m_modes.clear();

    // make sure fid is valid
    if( !this->isOpen() ) log( "Unable to call enumVideoModes() as device is NOT open", warning );

    else
    {
        // libusb stuff goes here eventually

        // TEST create a simple video mode for now
        this->m_modes[offset].fourcc = 1000;
        this->m_modes[offset].format_str = "Motion JPEG";
        this->m_modes[offset].size = 1920 * 4 * 1080;
        this->m_modes[offset].width = 1920;
        this->m_modes[offset++].height = 1080;

        ret = true;

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
