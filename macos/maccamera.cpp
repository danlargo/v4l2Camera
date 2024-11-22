#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

#include <unistd.h>
#include <fcntl.h>

#include "maccamera.h"
#include "v4l2_defs.h"
#include "libuvc/libuvc.h"

MACCamera::MACCamera( struct uvc_device * dev )
    : V4l2Camera()
{
    // save the device information
    m_dev = dev;

    m_userName = dev->productName;
    m_Handle = nullptr;

    m_cameraType = "MAC USB Camera";

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
        tmpC->setLogMode( v4l2_logging_mode::logOff );

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

                    if( tmpC->getVideoModes().size() > 0 )
                    {
                        // save the camera for use later
                        keep = true;
                        camList.push_back(tmpC);

                    } else tmpC->log( nam + " : zero video modes detected", info );
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
            log( "Failed to open UVC camera : " + std::string(uvc_strerror(err)),  v4l2_msg_type::critical );
            m_Handle = nullptr;

            // check for special case, access denied on MACOS...console apps CAN NOT request camera persmissions, they must be run as ROOT
            // sudo ./v4l2cam -l            ... if this lists cameras then you are good to go
            //
            #ifdef __APPLE__
                // this is likely a permisssions issue
                if(UVC_ERROR_ACCESS == err ) log( "You are running on MACOS, console apps CAN NOT request camera permissions, run as root", v4l2_msg_type::critical ); 
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

        // libuvc stuff goes here eventually 
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


bool MACCamera::setFrameFormat( struct v4l2_video_mode vm )
{
    // just save the requested mode as we don;t set it until we request the image frame
    m_currentMode = vm;

    return true;
}


bool MACCamera::init( enum v4l2_fetch_mode newMode )
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

struct v4l2_image_buffer * MACCamera::fetch( bool lastOne )
{
    struct v4l2_image_buffer * retBuffer = nullptr;

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
                struct v4l2_video_mode tmpMode;

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


        // Scanning Mode
        if( uvc_get_scanning_mode(m_Handle, &mode, UVC_GET_CUR) == UVC_SUCCESS )
        {
            struct v4l2_control tmpC;
            tmpC.name = "SCANNING_MODE";
            tmpC.type = v4l2_control_type::v4l2_boolean;
            tmpC.typeStr = "BOOLEAN";

            tmpC.value = mode;
            tmpC.min = 0;
            tmpC.max = 1;
            tmpC.step = 1;
            m_controls[ 0 ] = tmpC;
        }

        
    
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
    controlDefs[UVC_CT_SCANNING_MODE_CONTROL] = { UVC_CT_SCANNING_MODE_CONTROL, v4l2_control_type::v4l2_boolean,v4l2_control_unit::v4l2_terminal_unit, "Scanning Mode (Interlaced or Progressive)", 1, {} };
    controlDefs[UVC_CT_AE_MODE_CONTROL] = { UVC_CT_AE_MODE_CONTROL, v4l2_control_type::v4l2_menu, v4l2_control_unit::v4l2_terminal_unit, "AE_MODE", 1, { {1,"Auto"}, {2,"Manual"}, {4,"Shutter Priority"}, {8,"ApeturePriority"} } };
    controlDefs[UVC_CT_AE_PRIORITY_CONTROL] = { UVC_CT_AE_PRIORITY_CONTROL, v4l2_control_type::v4l2_menu, v4l2_control_unit::v4l2_terminal_unit, "AE_PRIORITY", 1, { {0,"Static"}, {1,"Dynamic"} } };
    controlDefs[UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL] = { UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_terminal_unit, "Exposure Time Absolute", 4, {} };
    controlDefs[UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL] = { UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_terminal_unit, "Exposure Time Relative", 1, {} };
    controlDefs[UVC_CT_FOCUS_ABSOLUTE_CONTROL] = { UVC_CT_FOCUS_ABSOLUTE_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_terminal_unit, "Focus Absolute (mm)", 2, {} };
    //controlDefs[UVC_CT_FOCUS_RELATIVE_CONTROL] = { UVC_CT_FOCUS_RELATIVE_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_unit, "Focus Relative", 1, {} };
    controlDefs[UVC_CT_FOCUS_RELATIVE_CONTROL] = { UVC_CT_FOCUS_RELATIVE_CONTROL, v4l2_control_type::v4l2_menu, v4l2_control_unit::v4l2_terminal_unit, "Focus Relative", 1, { {0x00, "Full Range"}, {0x01, "Macro"}, {0x02, "People"}, {0x03, "Scene"} } };
    controlDefs[UVC_CT_FOCUS_AUTO_CONTROL] = { UVC_CT_FOCUS_AUTO_CONTROL, v4l2_control_type::v4l2_boolean, v4l2_control_unit::v4l2_terminal_unit, "Focus Auto (Off or On)", 1, {} };
    controlDefs[UVC_CT_IRIS_ABSOLUTE_CONTROL] = { UVC_CT_IRIS_ABSOLUTE_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_terminal_unit, "Iris Absolute (fStop)", 2, {} };
    controlDefs[UVC_CT_IRIS_RELATIVE_CONTROL] = { UVC_CT_IRIS_RELATIVE_CONTROL, v4l2_control_type::v4l2_menu, v4l2_control_unit::v4l2_terminal_unit, "Iris Relative", 1, {{0, "Default"}, {1, "Open 1 Step"}, {0xff,"Close 1 Step"} } };
    controlDefs[UVC_CT_ZOOM_ABSOLUTE_CONTROL] = { UVC_CT_ZOOM_ABSOLUTE_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_terminal_unit, "Zoom Absolute", 2, {} };
    //controlDefs[UVC_CT_ZOOM_RELATIVE_CONTROL] = { UVC_CT_ZOOM_RELATIVE_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_unit, "Zoom Relative", 3, {} };
    //controlDefs[UVC_CT_PANTILT_ABSOLUTE_CONTROL] = { UVC_CT_PANTILT_ABSOLUTE_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_unit, "PanTilt Absolute", 8, {} };
    //controlDefs[UVC_CT_PANTILT_RELATIVE_CONTROL] = { UVC_CT_PANTILT_RELATIVE_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_unit, "PanTilt Relative", 4, {} };
    controlDefs[UVC_CT_ROLL_ABSOLUTE_CONTROL] = { UVC_CT_ROLL_ABSOLUTE_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_terminal_unit, "Roll Absolute", 2, {} };
    //controlDefs[UVC_CT_ROLL_RELATIVE_CONTROL] = { UVC_CT_ROLL_RELATIVE_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_unit, "Roll Relative", 2, {} };
    controlDefs[UVC_CT_PRIVACY_CONTROL] = { UVC_CT_PRIVACY_CONTROL, v4l2_control_type::v4l2_boolean, v4l2_control_unit::v4l2_terminal_unit, "Privacy Control (Off or On)", 1, {} };
    //controlDefs[UVC_CT_DIGITAL_WINDOW_CONTROL] = { UVC_CT_DIGITAL_WINDOW_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_unit, "Digital Window", 12, {} };
    //controlDefs[UVC_CT_REGION_OF_INTEREST_CONTROL] = { UVC_CT_REGION_OF_INTEREST_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_unit, "Region of Interest", 10, {} };

    // Processing Units Controls
    controlDefs[UVC_PU_BACKLIGHT_COMPENSATION_CONTROL] = { UVC_PU_BACKLIGHT_COMPENSATION_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_processing_unit, "Backlight Compensation", 2, {} };
    controlDefs[UVC_PU_BRIGHTNESS_CONTROL] = { UVC_PU_BRIGHTNESS_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_processing_unit, "Brightness", 2, {} };
    controlDefs[UVC_PU_CONTRAST_CONTROL] = { UVC_PU_CONTRAST_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_processing_unit, "Contrast", 2, {} };
    controlDefs[UVC_PU_CONTRAST_AUTO_CONTROL] = { UVC_PU_CONTRAST_AUTO_CONTROL, v4l2_control_type::v4l2_boolean, v4l2_control_unit::v4l2_processing_unit, "Contrast Auto (Off or On)", 1, {} };
    controlDefs[UVC_PU_GAIN_CONTROL] = { UVC_PU_GAIN_CONTROL, v4l2_control_type::v4l2_integer, v4l2_control_unit::v4l2_processing_unit, "Gain", 2, {} };
}

