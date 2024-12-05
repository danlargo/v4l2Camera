#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

#include <unistd.h>
#include <fcntl.h>

#include "maccamera.h"

MACCamera::MACCamera( struct usb_device * dev )
    : V4l2Camera()
{
    // save the device information
    m_dev = dev;
    m_userName = dev->productName;

    m_Handle = nullptr;

    m_cameraType = "generic MACOS UVC";

}


MACCamera::~MACCamera()
{
    if( m_Handle ) libusb_close( m_Handle );

}


void MACCamera::initAPI()
{
    libusb_init( &USB_ctx );
}


void MACCamera::closeAPI()
{
    libusb_exit( USB_ctx );
}


std::vector< MACCamera *> MACCamera::discoverCameras()
{
    std::vector< MACCamera *> camList;
    int count = 0;

    std::vector<struct usb_device *> devList = MACCamera::buildCamList();

    for( const auto &x : devList )
    {
        bool keep = false;

        // create the camera object
        MACCamera * tmpC = new MACCamera(x);
        tmpC->setLogMode( v4l2cam_logging_mode::logOff );

        std::string nam = x->productName;

        // open the camera so we can query all its capabilities
        if( tmpC && tmpC->open() )
        {
            if( tmpC->enumCapabilities() )
            {
                if( tmpC->canFetch() )
                {
                    // have it query its own capabilities
                    tmpC->enumControls();
                    tmpC->enumVideoModes();
                    
                    // save the camera for use later
                    keep = true;

                    // add to list
                    camList.push_back(tmpC);

                } else tmpC->log( nam + " : does not support video capture", info  );
            } else tmpC->log( nam + " : unable to query capabilities", info  );

            // close the camera
            tmpC->close();

        } else tmpC->log( nam + " : failed to open device", info  );

        // delete the camera if we are not keeping it
        if( !keep ) delete tmpC;
    }

    return camList;
}


std::vector<struct usb_device *> MACCamera::buildCamList()
{
    std::vector<struct usb_device *> ret;

    // grab a simple list of USB devices to start our libusb journey
    libusb_device **devs;
    int r;
    ssize_t cnt;

    cnt = libusb_get_device_list(NULL, &devs);
    //std::cerr << "[libusb] Found " << cnt << " usb devices" << std::endl;
    if (cnt >= 0)
    {
        libusb_device *dev;
        int i = 0, j = 0;
        uint8_t path[8];

        while ((dev = devs[i++]) != NULL) {
            struct libusb_device_descriptor desc;
            libusb_device_handle *hHandle;
            int r = libusb_get_device_descriptor(dev, &desc);
            if (r < 0)
            {
                std::cerr << "[libusb error] Failed to get device descriptor" << std::endl;
                break;
            }

            //std::cerr << "[libusb] " << desc.idVendor << "-" << desc.idProduct << "-" <<
            //                        std::to_string(libusb_get_bus_number(dev)) << "-" << std::to_string(libusb_get_device_address(dev))
            //                        << std::endl;

            int iReturnValue = libusb_open(dev, &hHandle);
            if (iReturnValue == LIBUSB_SUCCESS)
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
                        if ( 0x199e == desc.idVendor && desc.idProduct  >= 0x8201 && desc.idProduct <= 0x8208 )
                        {
                            std::cerr << "[libusb info] Skipping TIS Camera, whatever that is" << std::endl;
                            continue;
                        }

                        // Special case for Imaging Source cameras
                        /* Video, Streaming */
                        if ( 0x199e == desc.idVendor && ( 0x8101 == desc.idProduct || 0x8102 == desc.idProduct ) && if_desc->bInterfaceClass == 255 && if_desc->bInterfaceSubClass == 2 ) {
                            got_interface = 1;
                        }

                        /* Video, Streaming */
                        if (if_desc->bInterfaceClass == 14 && if_desc->bInterfaceSubClass == 2) {
                            got_interface = 1;
                        }
                    }
                }

                libusb_free_config_descriptor(config);

                unsigned char uProductName[255] = {};
                libusb_get_string_descriptor_ascii(hHandle, desc.iProduct, uProductName, sizeof(uProductName));
                //std::cerr << "[libusb] " << reinterpret_cast<char*>(uProductName);

                unsigned char uVendorName[255] = {};
                libusb_get_string_descriptor_ascii(hHandle, desc.idVendor, uVendorName, sizeof(uVendorName));
                //std::cerr << "[libusb] " << reinterpret_cast<char*>(uVendorName);

                if( got_interface ) 
                {
                    // save the data
                    struct usb_device * tmp = new struct usb_device;
                    tmp->bus = libusb_get_bus_number(dev);
                    tmp->address = libusb_get_device_address(dev);
                    tmp->vid = desc.idVendor;
                    tmp->pid = desc.idProduct;
                    tmp->productName = reinterpret_cast<char*>(uProductName);
                    tmp->manufacturerName = reinterpret_cast<char*>(uVendorName);
                    ret.push_back(tmp);

                    //std::cerr << " : This is a UVC camera" << std::endl;
                }

                // close the device handler
                libusb_close(hHandle);
            }
        }
        // we unreference one at a time, not the entire list
        libusb_free_device_list(devs, 1);
    }

    return ret;
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
    libusb_device **devs;
    int r;
    ssize_t cnt;

    // get the complete list each time, open specific device, then free list
    cnt = libusb_get_device_list(NULL, &devs);

    if (cnt >= 0)
    {
        libusb_device *dev;
        int i = 0, j = 0;
        uint8_t path[8];

        while ((dev = devs[i++]) != NULL) 
        {
            struct libusb_device_descriptor desc;
            int r = libusb_get_device_descriptor(dev, &desc);
            if (r < 0)
            {
                std::cerr << "[libusb error] Failed to get device descriptors" << std::endl;
                break;
            }

            // find the one we want
            if( m_dev->bus ==  libusb_get_bus_number(dev) && 
                m_dev->address == libusb_get_device_address(dev) &&
                m_dev->vid == desc.idVendor && m_dev->pid == desc.idProduct )
            {
                r = libusb_open(dev, &m_Handle);
                if (r == LIBUSB_SUCCESS)
                {
                    // indicate we are open ok
                    ret = true;
                    m_healthCounter = 0;
                }
                else
                {
                    m_healthCounter = s_healthCountLimit;
                    log( "Failed to open UVC camera : " + m_dev->productName,  v4l2cam_msg_type::critical );
                    m_Handle = nullptr;

                    // check for special case, access denied on MACOS...console apps CAN NOT request camera persmissions, they must be run as ROOT
                    // sudo ./v4l2cam -l            ... if this lists cameras then you are good to go
                    //
                    #ifdef __APPLE__
                        // this is likely a permisssions issue
                        log( "If you are running on MACOS, console apps CAN NOT request camera permissions, try  running as root", v4l2cam_msg_type::critical ); 
                    #endif

                    break;
                }
            }

        }
        libusb_free_device_list(devs, 1);

        // make sure we actually opened the device
        if( !ret )
        {

        }
    } else log( "No devices found in system : " + m_dev->productName + "-" + m_dev->manufacturerName, v4l2cam_msg_type::critical );

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

        // libuvc stuff goes here eventually 
        m_capabilities = 0;

        ret = true;
    }

    return ret;
}


void MACCamera::close()
{
    if( m_Handle != nullptr ) libusb_close( m_Handle );
    m_Handle = nullptr;
}


bool MACCamera::setFrameFormat( struct v4l2cam_video_mode vm )
{
    // just save the requested mode as we don;t set it until we request the image frame
    m_currentMode = vm;

    return true;
}


bool MACCamera::init( enum v4l2cam_fetch_mode newMode )
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

struct v4l2cam_image_buffer * MACCamera::fetch( bool lastOne )
{
    struct v4l2cam_image_buffer * retBuffer = nullptr;

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
                struct v4l2cam_video_mode tmpMode;

                tmpMode.fourcc = V4l2Camera::fourcc_charArray_to_int( (unsigned char*)format_desc->fourccFormat );
                tmpMode.format_str = V4l2Camera::fourcc_int_to_descriptor( tmpMode.fourcc );
                tmpMode.width = frame_desc->wWidth;
                tmpMode.height = frame_desc->wHeight;
                tmpMode.size = frame_desc->dwBytesPerLine * frame_desc->wHeight;

                // save all the info
                m_modes.push_back( tmpMode );

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
    m_controls.clear();

    // make sure fid is valid
    if( !isOpen() ) log( "Unable to call enumControls() as device is NOT open", warning );

    else
    {
        // walk through all the control functions to see which ones are supported
        unsigned char mode;

        // Data driven
        // - structure specifies CTRL_ID, Control Unit( for Camera Controls or Processing Controls), String value of control name, length of return values (1, 2, 4 for char, short or int)
        //
        // - setp 1 - query using uvc_get_ctrl, with type of GET_INFO, returns a capability flag (assume if not 0, then supported)
        // - step 2 - If supported
        //          - Check "ctrl_type"
        //          - if BOOLEAN, then only query GET_CUR, set min = 0, max = 1, step = 1, control id name should be descriptive, bool is always 0 = OFF/FALSE, 1 = ON/TRUE
        //          - if INTEGER, query CUR, MIN, MAX and RES
        //          - if MENU, query CUR, RES (which will be a bitmap of supported modes, assign hardcoded menu items for supported modes), set MIN and MAX based on bitmap, set RES = 1
        //          - structure will contain menu item strings
        //
        // If Unit = v4l2_terminal then uvc_get_ctrl -> unit = uvc_get_camera_terminal(devh)->bTerminalID
        // If Unit = v4l2_unit then uvc_get_ctrl -> unit = uvc_get_processing_units(devh)->bUnitID
        //
        
    
        ret = true;
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

void MACCamera::buildControlDefList()
{
    // Data driven - control structure
    //
    // - structure specifies CTRL_ID, Control Unit( for Camera Controls or Processing Controls), String value of control name, length of return values (1, 2, 4 for char, short or int)
    //
    // - setp 1 - query using uvc_get_ctrl, with type of GET_INFO, returns a capability flag (assume if not 0, then supported)
    // - step 2 - If supported
    //          - Check "ctrl_type"
    //          - if BOOLEAN, then only query GET_CUR, set min = 0, max = 1, step = 1, control id name should be descriptive, bool is always 0 = OFF/FALSE, 1 = ON/TRUE
    //          - if INTEGER, query CUR, MIN, MAX and RES
    //          - if MENU, query CUR, RES (which will be a bitmap of supported modes, assign hardcoded menu items for supported modes), set MIN and MAX based on bitmap, set RES = 1
    //          - structure will contain menu item strings
    //
    // If Unit = v4l2_terminal then uvc_get_ctrl -> terminal = uvc_get_camera_terminal(devh)->bTerminalID
    // If Unit = v4l2_unit then uvc_get_ctrl -> processing = uvc_get_processing_units(devh)->bUnitID
    //

    // clear the list
    controlDefs.clear();

    // any control commented out below needs special handling as it has more than one modifier value
    //
    // Terminal Control Requests
    //
    /*
    controlDefs[UVC_CT_SCANNING_MODE_CONTROL] = { UVC_CT_SCANNING_MODE_CONTROL, v4l2cam_control_type::v4l2_boolean,v4l2cam_control_unit::v4l2_terminal_unit, "Scanning Mode (Interlaced or Progressive)", 1, {} };
    controlDefs[UVC_CT_AE_MODE_CONTROL] = { UVC_CT_AE_MODE_CONTROL, v4l2cam_control_type::v4l2_menu, v4l2cam_control_unit::v4l2_terminal_unit, "AE_MODE", 1, { {1,"Auto"}, {2,"Manual"}, {4,"Shutter Priority"}, {8,"ApeturePriority"} } };
    controlDefs[UVC_CT_AE_PRIORITY_CONTROL] = { UVC_CT_AE_PRIORITY_CONTROL, v4l2cam_control_type::v4l2_menu, v4l2cam_control_unit::v4l2_terminal_unit, "AE_PRIORITY", 1, { {0,"Static"}, {1,"Dynamic"} } };
    controlDefs[UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL] = { UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_terminal_unit, "Exposure Time Absolute", 4, {} };
    controlDefs[UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL] = { UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_terminal_unit, "Exposure Time Relative", 1, {} };
    controlDefs[UVC_CT_FOCUS_ABSOLUTE_CONTROL] = { UVC_CT_FOCUS_ABSOLUTE_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_terminal_unit, "Focus Absolute (mm)", 2, {} };
    //controlDefs[UVC_CT_FOCUS_RELATIVE_CONTROL] = { UVC_CT_FOCUS_RELATIVE_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_unit, "Focus Relative", 1, {} };
    controlDefs[UVC_CT_FOCUS_RELATIVE_CONTROL] = { UVC_CT_FOCUS_RELATIVE_CONTROL, v4l2cam_control_type::v4l2_menu, v4l2cam_control_unit::v4l2_terminal_unit, "Focus Relative", 1, { {0x00, "Full Range"}, {0x01, "Macro"}, {0x02, "People"}, {0x03, "Scene"} } };
    controlDefs[UVC_CT_FOCUS_AUTO_CONTROL] = { UVC_CT_FOCUS_AUTO_CONTROL, v4l2cam_control_type::v4l2_boolean, v4l2cam_control_unit::v4l2_terminal_unit, "Focus Auto (Off or On)", 1, {} };
    controlDefs[UVC_CT_IRIS_ABSOLUTE_CONTROL] = { UVC_CT_IRIS_ABSOLUTE_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_terminal_unit, "Iris Absolute (fStop)", 2, {} };
    controlDefs[UVC_CT_IRIS_RELATIVE_CONTROL] = { UVC_CT_IRIS_RELATIVE_CONTROL, v4l2cam_control_type::v4l2_menu, v4l2cam_control_unit::v4l2_terminal_unit, "Iris Relative", 1, {{0, "Default"}, {1, "Open 1 Step"}, {0xff,"Close 1 Step"} } };
    controlDefs[UVC_CT_ZOOM_ABSOLUTE_CONTROL] = { UVC_CT_ZOOM_ABSOLUTE_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_terminal_unit, "Zoom Absolute", 2, {} };
    //controlDefs[UVC_CT_ZOOM_RELATIVE_CONTROL] = { UVC_CT_ZOOM_RELATIVE_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_unit, "Zoom Relative", 3, {} };
    //controlDefs[UVC_CT_PANTILT_ABSOLUTE_CONTROL] = { UVC_CT_PANTILT_ABSOLUTE_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_unit, "PanTilt Absolute", 8, {} };
    //controlDefs[UVC_CT_PANTILT_RELATIVE_CONTROL] = { UVC_CT_PANTILT_RELATIVE_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_unit, "PanTilt Relative", 4, {} };
    controlDefs[UVC_CT_ROLL_ABSOLUTE_CONTROL] = { UVC_CT_ROLL_ABSOLUTE_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_terminal_unit, "Roll Absolute", 2, {} };
    //controlDefs[UVC_CT_ROLL_RELATIVE_CONTROL] = { UVC_CT_ROLL_RELATIVE_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_unit, "Roll Relative", 2, {} };
    controlDefs[UVC_CT_PRIVACY_CONTROL] = { UVC_CT_PRIVACY_CONTROL, v4l2cam_control_type::v4l2_boolean, v4l2cam_control_unit::v4l2_terminal_unit, "Privacy Control (Off or On)", 1, {} };
    //controlDefs[UVC_CT_DIGITAL_WINDOW_CONTROL] = { UVC_CT_DIGITAL_WINDOW_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_unit, "Digital Window", 12, {} };
    //controlDefs[UVC_CT_REGION_OF_INTEREST_CONTROL] = { UVC_CT_REGION_OF_INTEREST_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_unit, "Region of Interest", 10, {} };

    // Processing Units Controls
    controlDefs[UVC_PU_BACKLIGHT_COMPENSATION_CONTROL] = { UVC_PU_BACKLIGHT_COMPENSATION_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_processing_unit, "Backlight Compensation", 2, {} };
    controlDefs[UVC_PU_BRIGHTNESS_CONTROL] = { UVC_PU_BRIGHTNESS_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_processing_unit, "Brightness", 2, {} };
    controlDefs[UVC_PU_CONTRAST_CONTROL] = { UVC_PU_CONTRAST_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_processing_unit, "Contrast", 2, {} };
    controlDefs[UVC_PU_CONTRAST_AUTO_CONTROL] = { UVC_PU_CONTRAST_AUTO_CONTROL, v4l2cam_control_type::v4l2_boolean, v4l2cam_control_unit::v4l2_processing_unit, "Contrast Auto (Off or On)", 1, {} };
    controlDefs[UVC_PU_GAIN_CONTROL] = { UVC_PU_GAIN_CONTROL, v4l2cam_control_type::v4l2_integer, v4l2cam_control_unit::v4l2_processing_unit, "Gain", 2, {} };
*/
}

