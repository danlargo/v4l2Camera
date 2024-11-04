#include <cstring>
#include <string>
#include <iostream>
#include <vector>

// using ioctl for low level device enumeration and control
#include <sys/ioctl.h>

#ifdef __linux__
    #include <linux/videodev2.h>
    
#elif __APPLE__
    #include <libusb-1.0/libusb.h>
#endif

#include <unistd.h>
#include <fcntl.h>

#include "v4l2camera.h"

#ifdef __linux__
V4l2Camera::V4l2Camera( std::string device_name )
{
    this->m_fidName = device_name;
    this->m_userName = "";
    this->m_capabilities = 0;
    this->m_fid = -1;
    this->m_bufferMode = notset;

    // default logging, keep internal buffer, 500 entries deep
    this->m_logMode = logInternal;
    clearLog();

    // initiallize the return buffer
    this->m_frameBuffer = nullptr;
}

#elif __APPLE__
V4l2Camera::V4l2Camera( struct usb_device * dev )
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
#endif

V4l2Camera::~V4l2Camera()
{
    // close the device before we disappear - if m_fid is set then the device is likely open
    #ifdef __linux__
        if( this->m_fid > -1 ) ::close(this->m_fid);
    #elif __APPLE__
        if( m_Handle ) libusb_close(m_Handle);
    #endif
}

void V4l2Camera::initAPI()
{
    #ifdef __APPLE__
        libusb_init(NULL);
    #endif
}

void V4l2Camera::closeAPI()
{
    #ifdef __APPLE__
        libusb_exit(NULL);
    #endif
}

std::map<int, V4l2Camera *> V4l2Camera::discoverCameras()
{
    std::map<int, V4l2Camera *> camList;
    int count = 0;

    #ifdef __linux__
        std::vector<std::string> devList = V4l2Camera::buildCamList_dev();
    #elif __APPLE__
        std::vector<struct usb_device *> devList = V4l2Camera::buildCamList_usb();
    #endif

    for( const auto &x : devList )
    {
        bool keep = false;

        // create the camera object
        V4l2Camera * tmpC = new V4l2Camera(x);
        tmpC->setLogMode( logToStdOut );

        #ifdef __linux__
            std::string nam = x;
        #elif __APPLE__
            std::string nam = x->productName;
        #endif

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

#ifdef __linux__
std::vector<std::string> V4l2Camera::buildCamList_dev()
{
    std::vector<std::string> ret;
    int maxCams = 64;

    std::cout << "Building list of potential UVC Cameras (using /dev/video*)" << std::endl;

    for( int i=0;i<maxCams;i++ ) ret.push_back( "/dev/video" + std::to_string(i) );

    std::cout << "Added " << maxCams << " potential devices" << std::endl;

    return ret;
}

#elif __APPLE__
std::vector<struct usb_device *> V4l2Camera::buildCamList_usb()
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
                        if ( 0x199e == desc.idVendor && ( 0x8101 == desc.idProduct || 0x8102 == desc.idProduct ) && if_desc->bInterfaceClass == 255 && if_desc->bInterfaceSubClass == 2 ) 
                        {
                            got_interface = 1;
                        }

                        // UVC - Video, Streaming Device
                        if (if_desc->bInterfaceClass == 14 && if_desc->bInterfaceSubClass == 2) got_interface = 1;
                    }
                }

                libusb_free_config_descriptor(config);

                if( got_interface ) 
                {
                    unsigned char str_product[255] = {};
                    unsigned char str_manuf[255] = {};
                    libusb_get_string_descriptor_ascii(hHandle, desc.iProduct, str_product, sizeof(str_product));
                    libusb_get_string_descriptor_ascii(hHandle, desc.iManufacturer, str_manuf, sizeof(str_manuf));

                    // save the information
                    struct usb_device * newDev = new struct usb_device;
                    newDev->bus = bus_num;
                    newDev->address = dev_addr;
                    newDev->vid = desc.idVendor;
                    newDev->pid = desc.idProduct;
                    newDev->productName = (char *)str_product;
                    newDev->manufacturerName = (char *)str_manuf;
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

libusb_device_handle * V4l2Camera::openCamera_usb( int bus, int address, int vid, int pid )
{
    libusb_device_handle * ret = nullptr;

    libusb_device **devs;
    if( libusb_get_device_list(NULL, &devs) >= 0)
    {
        libusb_device *dev;
        int i = 0, j = 0;
        uint8_t path[8];

        while ((dev = devs[i++]) != NULL) 
        {
            struct libusb_device_descriptor desc;
            if( libusb_get_device_descriptor(dev, &desc) < 0) continue;

            int bus_num = libusb_get_bus_number(dev);
            int dev_addr = libusb_get_device_address(dev);

            if( bus_num == bus && dev_addr == address && vid == desc.idVendor && pid == desc.idProduct )
            {
                libusb_device_handle * hHandle;
                if( libusb_open(dev, &hHandle) == LIBUSB_SUCCESS ) ret = hHandle;
                // this is the ONE, stop looking
                break;
            }
        }
        libusb_free_device_list(devs, 1);
    }

    return ret;
}


#endif

std::string V4l2Camera::getCameraType()
{
    return "generic";
}

std::string V4l2Camera::getFidName()
{
    #ifdef __linux__
        return this->m_fidName;
    #elif __APPLE__
        if( m_dev )
        {
            return std::to_string(m_dev->bus) + "." + std::to_string(m_dev->address) + "." + std::to_string(m_dev->vid) + "." + std::to_string(m_dev->pid);
        } else return "USB Device, address not set";
    #endif
}

std::string V4l2Camera::getUserName()
{
    return this->m_userName;
}

bool V4l2Camera::canOpen()
{
#ifdef __linux__
    int fd = ::open(this->m_fidName.c_str(), O_RDWR);
    ::close(fd);

    return( fd > -1 );
#elif __APPLE__
    return true;
#endif

}

bool V4l2Camera::canFetch()
{
    #ifdef __linux__
        return ( this->m_capabilities & V4L2_CAP_VIDEO_CAPTURE );
    #elif __APPLE__
        return true;
    #endif
    
}

bool V4l2Camera::canRead()
{
    #ifdef __linux__
        return ( this->m_capabilities & V4L2_CAP_READWRITE );
    #elif __APPLE__
        return false;
    #endif

}

void V4l2Camera::setLogMode( enum logging_mode newMode )
{
    this->m_logMode = newMode;
}

void V4l2Camera::log( std::string out, enum msg_type tag )
{
    if( logOff != this->m_logMode )
    {
        // build the log message
        std::string msg = "[" + this->getTagStr(tag) + "] " + this->getFidName() +  " : " + out;

        // immediately add to the end of the list
        this->m_debugLog.push_back( msg );

        // check if we should do anything else with it
        if( logToStdErr == this->m_logMode ) std::cerr << msg << std::endl;
        if( logToStdOut == this->m_logMode ) std::cout << msg << std::endl;

        // check the length of the log, if greater than logDepth then clean from beginning
        while( this->m_debugLog.size() > this->s_logDepth ) this->m_debugLog.erase(this->m_debugLog.begin() );

    } else this->m_debugLog.clear();       // make sure log is off and empty
}

std::string V4l2Camera::getTagStr( enum msg_type tag )
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

void V4l2Camera::clearLog()
{
    this->m_debugLog.clear();
}

int V4l2Camera::setValue( int id, int newVal, bool openOnDemand )
{
    bool closeOnExit = false;
    int ret = -1;

    #ifdef __linux__
        struct v4l2_control outQuery;

        // check if the device is open before trying to set the value
        if( !openOnDemand && (-1 == this->m_fid) )
        {
            log( "Unable to call setValue() as device is NOT open", warning );
            return -1;
        }

        // if device is closed, check if we ae being asked to open it
        if( openOnDemand && (-1 == this->m_fid) )
        {
            if( !this->open() )
            {
                log( "Unable to openOnDemand for setValue() : " + std::string(strerror(errno)), error );
                return -1;
            }
            closeOnExit = true;
        }

        memset( &outQuery, 0, sizeof(struct v4l2_control));
        outQuery.id = id;
        outQuery.value = newVal;
        ret = ioctl(this->m_fid, VIDIOC_S_CTRL, &outQuery );

        if( -1 == ret ) log( "ioctl(VIDIOC_S_CTRL) [" + std::to_string(id) + "] failed :  " + strerror(errno), info );
        else log( "ioctl(VIDIOC_S_CTRL) [" + std::to_string(id) + "] = " + std::to_string(newVal), info );

        if( closeOnExit )
        {
            ::close( this->m_fid );
            this->m_fid = -1;
        }

    #endif

    return ret;
}

int V4l2Camera::getValue( int id, bool openOnDemand )
{

    bool closeOnExit = false;
    int ret = -1;

    #ifdef __linux__
        struct v4l2_control outQuery;

        // check if device is open
        if( !openOnDemand && (-1 == this->m_fid) )
        {
            log( "Unable to call getValue() as device is NOT open", warning );
            return -1;
        }

        // check if we are asked to open it
        if( openOnDemand && (-1 == this->m_fid) )
        {
            if( !this->open() )
            {
                log( "Unable to openOnDemand for getValue() : " + std::string(strerror(errno)), error );
                return -1;
            }
            closeOnExit = true;
        }

        // get the actual value
        memset( &outQuery, 0, sizeof(struct v4l2_control));
        outQuery.id = id;
        if( -1 == ioctl(m_fid, VIDIOC_G_CTRL, &outQuery ) ) log( "ioctl(VIDIOC_G_CTRL) [" + std::to_string(id) + "] failed :  " + strerror(errno), info );
        else
        {
            ret = outQuery.value;
            log( "ioctl(VIDIOC_G_CTRL) [" + std::to_string(id) +"] = " + std::to_string(ret), info );
        }

        // close it if we opened on demand
        if( closeOnExit )
        {
            ::close(this->m_fid);
            this->m_fid = -1;
        }

    #endif

    return ret;
}

bool V4l2Camera::open()
{
    bool ret = false;

    #ifdef __linux__
        this->m_fid = ::open(this->m_fidName.c_str(), O_RDWR);

        if( -1 == this->m_fid ) log( "::open(" + this->m_fidName + ") failed : " + std::string(strerror(errno)), error );

        else
        {
            // set the fetch mode to USERPtr as a default, UNTIL WE SUPPORT MMAP
            this->m_bufferMode = userPtrMode;

            // indicate we are open ok
            ret = true;
            log( "::open(" + this->m_fidName + ") success, FID = " + std::to_string(this->m_fid), info );
        }

    #elif __APPLE__
        libusb_device **devs;
        if( libusb_get_device_list(NULL, &devs) >= 0)
        {
            libusb_device *dev;
            int i = 0, j = 0;
            uint8_t path[8];

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
    #endif

    return ret;
}

bool V4l2Camera::isOpen()
{
    #ifdef __linux__
        return( m_fid > -1 );
    #elif __APPLE__
        return( m_Handle != nullptr );
    #endif
}

bool V4l2Camera::enumCapabilities()
{
    bool ret = false;

    if( !this->isOpen() ) log( "Unable to call getCaps() as device is NOT open", warning );

    else
    {
        #ifdef __linux__
            struct v4l2_capability tmpV;

            if( -1 == ioctl(this->m_fid, VIDIOC_QUERYCAP, &tmpV) ) log( "ioctl(VIDIOC_QUERYCAP) failed :  " + std::string(strerror(errno)), error );

            else
            {
                // grab the device name
                this->m_userName = (char *)(tmpV.card);
                // truncate the name if it is duplicated
                int colon = this->m_userName.find(":");
                if(  colon > -1 ) this->m_userName = this->m_userName.substr(0,this->m_userName.find(":"));

                // save the capabilities vector
                this->m_capabilities = tmpV.capabilities;
                log( "ioctl(VIDIOC_QUERYCAP) " + this->m_fidName + " success, vector = " + std::to_string(this->m_capabilities), info );

                ret = true;
            }

        #elif __APPLE__
                // grab the device name
                this->m_userName = m_dev->manufacturerName + " " + m_dev->productName;
            ret = true;
        #endif
    }

    return ret;
}

bool V4l2Camera::checkCapabilities( unsigned int val )
{
        return ( this->m_capabilities & val );
}

void V4l2Camera::close()
{
    #ifdef _linux__
        enum v4l2_buf_type type;

        // disable streaming mode so the buffers are no longer being filled
        switch( this->m_bufferMode )
        {
            case notset:
            case readMode:
            case mMapMode:
                break;

            case userPtrMode:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if( -1 == ioctl( this->m_fid, VIDIOC_STREAMOFF, &type) ) log( "ioctl(VIDIOC_STREAMOFF) failed : " + std::string(strerror(errno)), error );
                else log( "VIDIOC_STREAMOFF success (streaming off) for " + this->m_fidName, info );
                break;
        }

        ::close(this->m_fid);
        this->m_fid = -1;

        log( this->m_fidName + " closed", info );

    #elif __APPLE__
        if( m_Handle != nullptr ) libusb_close(m_Handle);
        m_Handle = nullptr;
    #endif
}

bool V4l2Camera::setFrameFormat( struct video_mode vm )
{
    bool ret = false;

    #ifdef __linux__
        struct v4l2_format fmt;

        if( -1 == this->m_fid ) log( "Unable to call setFrameFormat() as device is NOT open", warning );
        {
            fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            fmt.fmt.pix.pixelformat = vm.fourcc;
            fmt.fmt.pix.width       = vm.width;
            fmt.fmt.pix.height      = vm.height;

            if( -1 == ioctl(this->m_fid, VIDIOC_S_FMT, &fmt) ) log( "ioctl(VIDIOC_S_FMT) failed : " + std::string(strerror(errno)), error );
            {
                this->m_currentMode = vm;
                ret = true;
                log( "ioctl(VIDIOC_S_FMT) success, set to : "
                        + vm.format_str + " at [" 
                        + std::to_string(vm.width) + " x " 
                        + std::to_string(vm.height) + "]", info );
            }
        }
    
    #endif

    return ret;
}

bool V4l2Camera::setFrameFormat( std::string mode, int width, int height )
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

bool V4l2Camera::init( enum fetch_mode newMode )
{
    bool ret = false;

    if( !this->isOpen() ) log( "Unable to call init() as device is NOT open", info );
    {
#ifdef __linux__
        this->m_bufferMode = newMode;

        switch( this->m_bufferMode )
        {
            case readMode:
                // do nothing
                ret = true;
                break;

            case userPtrMode:
                struct v4l2_requestbuffers req;

                memset(&req,0,sizeof(struct v4l2_requestbuffers));

                req.count  = 1;
                req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                req.memory = V4L2_MEMORY_USERPTR;

                if( -1 == ioctl(this->m_fid, VIDIOC_REQBUFS, &req) ) log( "ioctl(VIDIOC_REQBUF) failed : " + std::string(strerror(errno)), error );

                else
                {
                    log( "ioctl(VIDIOC_REQBUF) success", info );

                    // queuing up this->numBuffers fetch buffers
                    this->m_frameBuffer = new struct image_buffer;
                    this->m_frameBuffer->length =  this->m_currentMode.size;
                    this->m_frameBuffer->buffer = new unsigned char[this->m_currentMode.size];

                    // queue up the buffer
                    struct v4l2_buffer buf;

                    memset(&buf, 0, sizeof(struct v4l2_buffer));
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_USERPTR;
                    buf.index = 0;
                    buf.m.userptr = (unsigned long)(this->m_frameBuffer->buffer);
                    buf.length = this->m_frameBuffer->length;

                    if( -1 == ioctl(this->m_fid, VIDIOC_QBUF, &buf) ) log( "ioctl(VIDIOC_QBUF) failed : " + std::string(strerror(errno)), error );

                    else
                    {
                        log( "ioctl(VIDIOC_QBUF) Q'd 1 buffer", info );

                        // turn streaming on
                        enum v4l2_buf_type type;
                        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        if( -1 == ioctl(this->m_fid, VIDIOC_STREAMON, &type) ) log( "ioctl(VIDIOC_STREAMON) failed : " + std::string(strerror(errno)), error );

                        else
                        {
                            log( "ioctl(VIDIOC_STREAMON) success", info );
                            ret = true;
                        }
                    }
                }

                break;

            case mMapMode:
            case notset:
                break;
        }
#endif

    }

    return ret;
}

struct image_buffer * V4l2Camera::fetch( bool lastOne )
{
    struct image_buffer * retBuffer = nullptr;

    if( !this->isOpen() ) log( "Unable to call fetch() as no device is open", warning );
    {
        #ifdef __linux__
        switch( this->m_bufferMode )
        {
            case readMode:
                // do nothing
                break;

            case userPtrMode:
                // dequeue one frame
                struct v4l2_buffer buf;
                memset(&buf, 0, sizeof(struct v4l2_buffer));

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;

                if( -1 == ioctl(this->m_fid, VIDIOC_DQBUF, &buf) ) log( "ioctl(VIDIOC_DQBUF) failed : " + std::string(strerror(errno)), error );

                else
                {
                    retBuffer = new struct image_buffer;
                    // this should have de-queued into the previous buffer we allocated
                    retBuffer->buffer = this->m_frameBuffer->buffer;
                    retBuffer->length = buf.bytesused;

                    // only re-queue if we are going to be getting more
                    if( !lastOne )
                    {
                        // aloocating for this->numBuffers fetch buffers
                        this->m_frameBuffer = new struct image_buffer;
                        this->m_frameBuffer->length =  this->m_currentMode.size;
                        this->m_frameBuffer->buffer = new unsigned char[this->m_currentMode.size];

                        // queue up the first buffer
                        struct v4l2_buffer buf;

                        memset(&buf, 0, sizeof(struct v4l2_buffer));
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_USERPTR;
                        buf.index = 0;
                        buf.m.userptr = (unsigned long)(this->m_frameBuffer->buffer);
                        buf.length = this->m_frameBuffer->length;

                        if( -1 == ioctl(this->m_fid, VIDIOC_QBUF, &buf) ) log( "ioctl(VIDIOC_QBUF) failed : " + std::string(strerror(errno) ), error );
                    }
                }
                break;

            case mMapMode:
            case notset:
                break;
        }
        #endif
    }

    return retBuffer;
}

struct video_mode V4l2Camera::getOneVM( int index )
{
    // check if it exists
    if( this->m_modes.find(index) == this->m_modes.end() ) throw std::runtime_error("Video Mode does not exist");
    else return this->m_modes[index];
}

bool V4l2Camera::enumVideoModes()
{
    int offset = 0;
    bool ret = false;

    // clear the existing video structure
    this->m_modes.clear();

    // make sure fid is valid
    if( !this->isOpen() ) log( "Unable to call enumVideoModes() as device is NOT open", warning );

    else
    {
        #ifdef __linux__
        struct v4l2_fmtdesc tmpF;
        struct v4l2_frmsizeenum tmpS;

        memset( &tmpF, 0, sizeof(tmpF) );
        memset( &tmpS, 0, sizeof(tmpS) );

        log( "ioctl(VIDIOC_ENUM_FMT) for : " + this->m_userName, info );

        // walk the list of pixel formats, for each format, walk the list of video modes (sizes)
        tmpF.index = 0;
        tmpF.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        while( -1 != ioctl(this->m_fid, VIDIOC_ENUM_FMT, &tmpF ) )
        {
            log( "ioctl(VIDIOC_ENUM_FRAMESIZES) for : " + std::string((char*)(tmpF.description)), info );

            // walk tje frame sizes for this pixel format
            tmpS.index = 0;
            tmpS.pixel_format = tmpF.pixelformat;
            while( -1 != ioctl( this->m_fid, VIDIOC_ENUM_FRAMESIZES, &tmpS ) )
            {
                log( "Found : " + std::to_string(tmpS.discrete.width) + " x " + std::to_string(tmpS.discrete.height), info );

                // save all the info
                this->m_modes[offset].fourcc = tmpF.pixelformat;
                this->m_modes[offset].format_str = (char*)(tmpF.description);
                this->m_modes[offset].size = tmpS.discrete.width * 4 * tmpS.discrete.height;
                this->m_modes[offset].width = tmpS.discrete.width;
                this->m_modes[offset++].height = tmpS.discrete.height;

                tmpS.index++;
            }
            tmpF.index++;
        }
        ret = true;

        #elif __APPLE__
        // create a simple video mode for now
        this->m_modes[offset].fourcc = 1000;
        this->m_modes[offset].format_str = "Motion JPEG";
        this->m_modes[offset].size = 1920 * 4 * 1080;
        this->m_modes[offset].width = 1920;
        this->m_modes[offset++].height = 1080;

        ret = true;
        #endif

    }

    return ret;
}

struct user_control V4l2Camera::getOneCntrl( int index )
{
    // check if it exists
    if( this->m_controls.find(index) == this->m_controls.end() ) throw std::runtime_error("Control does not exist");
    else return this->m_controls[index];
}

bool V4l2Camera::enumControls()
{
    bool ret = false;

    // clear the existing control structure
    this->m_controls.clear();

    // make sure fid is valid
    if( !this->isOpen() ) log( "Unable to call enumControls() as device is NOT open", warning );

    else
    {
        #ifdef __linux__
        log( "ioctl(VIDIOC_QUERY_EXT_CTRL) for : " + this->m_userName, info );

        // walk all the private extended controls starting at V4L2_CID_PRIVATE_BASE
        // call should fail when there are no more private controls
        struct v4l2_query_ext_ctrl query_ext_ctrl;

        memset(&query_ext_ctrl, 0, sizeof(query_ext_ctrl));

        query_ext_ctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
        while (-1 != ioctl(this->m_fid, VIDIOC_QUERY_EXT_CTRL, &query_ext_ctrl))
        {
            if (!(query_ext_ctrl.flags & V4L2_CTRL_FLAG_DISABLED))
            {
                if( query_ext_ctrl.type != V4L2_CTRL_TYPE_CTRL_CLASS )
                {
                    log( "Found : " + std::string(query_ext_ctrl.name) );

                    this->m_controls[query_ext_ctrl.id].name = (char*)(query_ext_ctrl.name);
                    this->m_controls[query_ext_ctrl.id].value = query_ext_ctrl.default_value;
                    this->m_controls[query_ext_ctrl.id].type = query_ext_ctrl.type;
                    this->m_controls[query_ext_ctrl.id].typeStr = cntrlTypeToString(query_ext_ctrl.type);
                    this->m_controls[query_ext_ctrl.id].min = query_ext_ctrl.minimum;
                    this->m_controls[query_ext_ctrl.id].max = query_ext_ctrl.maximum;
                    this->m_controls[query_ext_ctrl.id].step = query_ext_ctrl.step;

                    // if this is a menu then grab the menu items
                    struct v4l2_querymenu querymenu;
                    memset (&querymenu, 0, sizeof (querymenu));
                    querymenu.id = query_ext_ctrl.id;

                    log( "ioctl(VIDIOC_QUERYMENU) for : " + std::to_string(querymenu.id), info );

                    for (querymenu.index = query_ext_ctrl.minimum; querymenu.index <= query_ext_ctrl.maximum; querymenu.index++)
                    {
                        if (-1 != ioctl (this->m_fid, VIDIOC_QUERYMENU, &querymenu))
                        {
                            log( "Menu item : " + std::string((char*)(querymenu.name)) );
                            this->m_controls[query_ext_ctrl.id].menuItems[querymenu.index] = std::string((char*)(querymenu.name));
                        }
                    }

                } else log( "Skipping : " + std::string(query_ext_ctrl.name), info );
            }

            query_ext_ctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }
        ret = true;
        #endif

    }

    return ret;
}

std::string V4l2Camera::cntrlTypeToString( int type )
{
    std::string ret = "unknown";

    switch( type )
    {
        #ifdef __linux__
        case V4L2_CTRL_TYPE_INTEGER:
            ret = "int";
            break;
        case V4L2_CTRL_TYPE_BOOLEAN:
            ret = "bool";
            break;
        case V4L2_CTRL_TYPE_MENU:
            ret = "menu";
            break;
        case V4L2_CTRL_TYPE_INTEGER_MENU:
            ret = "int-menu";
            break;
        case V4L2_CTRL_TYPE_BITMASK:
            ret = "bitmask";
            break;
        case V4L2_CTRL_TYPE_BUTTON:
            ret = "button";
            break;
        case V4L2_CTRL_TYPE_INTEGER64:
            ret = "int64";
            break;
        case V4L2_CTRL_TYPE_STRING:
            ret = "str";
            break;
        case V4L2_CTRL_TYPE_CTRL_CLASS:
            ret = "cntrl-class";
            break;
        case V4L2_CTRL_TYPE_U8:
            ret = "uint8";
            break;
        case V4L2_CTRL_TYPE_U16:
            ret = "uint16";
            break;
        case V4L2_CTRL_TYPE_U32:
            ret = "uint32";
            break;
        #endif

        default:
            break;
    }

    return ret;
}
