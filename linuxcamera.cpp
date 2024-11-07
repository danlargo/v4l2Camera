#include <cstring>
#include <string>
#include <iostream>
#include <vector>

// using ioctl for low level device enumeration and control
#ifdef __linux__
    #include <sys/ioctl.h>
    #include <linux/videodev2.h>
#endif

#include <unistd.h>
#include <fcntl.h>

#include "linuxcamera.h"

LinuxCamera::LinuxCamera( std::string device_name )
    : V4l2Camera()
{
    m_devName = device_name;
    m_userName = "";
    m_capabilities = 0;
    m_fid = -1;
    m_bufferMode = notset;
}


LinuxCamera::~LinuxCamera()
{
    // close the device before we disappear - if m_fid is set then the device is likely open
    if( this->m_fid > -1 ) ::close(this->m_fid);
}


std::map<int, LinuxCamera *> LinuxCamera::discoverCameras()
{
    std::map<int, LinuxCamera *> camList;
    int count = 0;

    std::vector<std::string> devList = LinuxCamera::buildCamList();

    for( const auto &x : devList )
    {
        bool keep = false;

        // create the camera object
        LinuxCamera * tmpC = new LinuxCamera(x);
        tmpC->setLogMode( logToStdOut );

        std::string nam = x;

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


std::vector<std::string> LinuxCamera::buildCamList()
{
    std::vector<std::string> ret;
    int maxCams = 64;

    std::cout << "Building list of potential UVC Cameras (using /dev/video*)" << std::endl;

    for( int i=0;i<maxCams;i++ ) ret.push_back( "/dev/video" + std::to_string(i) );

    std::cout << "Added " << maxCams << " potential devices" << std::endl;

    return ret;
}


std::string LinuxCamera::getCameraType()
{
    return "generic";
}

std::string LinuxCamera::getDevName()
{
    return this->m_devName;
}


bool LinuxCamera::canOpen()
{
    int fd = ::open(this->m_devName.c_str(), O_RDWR);
    ::close(fd);

    return( fd > -1 );

}


bool LinuxCamera::canFetch()
{
    return ( this->m_capabilities & V4L2_CAP_VIDEO_CAPTURE );
}


bool LinuxCamera::canRead()
{
    return ( this->m_capabilities & V4L2_CAP_READWRITE );
}


int LinuxCamera::setValue( int id, int newVal, bool openOnDemand )
{
    bool closeOnExit = false;
    int ret = -1;

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

    return ret;
}


int LinuxCamera::getValue( int id, bool openOnDemand )
{

    bool closeOnExit = false;
    int ret = -1;

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

    return ret;
}


bool LinuxCamera::open()
{
    bool ret = false;

    this->m_fid = ::open(this->m_devName.c_str(), O_RDWR);

    if( -1 == this->m_fid ) log( "::open(" + this->m_devName + ") failed : " + std::string(strerror(errno)), error );

    else
    {
        // set the fetch mode to USERPtr as a default, UNTIL WE SUPPORT MMAP
        this->m_bufferMode = userPtrMode;

        // indicate we are open ok
        ret = true;
        log( "::open(" + this->m_devName + ") success, FID = " + std::to_string(this->m_fid), info );
    }

    return ret;
}


bool LinuxCamera::isOpen()
{
    return( m_fid > -1 );
}


bool LinuxCamera::enumCapabilities()
{
    bool ret = false;

    if( !this->isOpen() ) log( "Unable to call getCaps() as device is NOT open", warning );

    else
    {
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
            log( "ioctl(VIDIOC_QUERYCAP) " + this->m_devName + " success, vector = " + std::to_string(this->m_capabilities), info );

            ret = true;
        }
    }

    return ret;
}


void LinuxCamera::close()
{
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
            else log( "VIDIOC_STREAMOFF success (streaming off) for " + this->m_devName, info );
            break;
    }

    ::close(this->m_fid);
    this->m_fid = -1;

    log( this->m_devName + " closed", info );

}


bool LinuxCamera::setFrameFormat( struct video_mode vm )
{
    bool ret = false;

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
    
    return ret;
}


bool LinuxCamera::init( enum fetch_mode newMode )
{
    bool ret = false;

    if( !this->isOpen() ) log( "Unable to call init() as device is NOT open", info );
    {
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
    }

    return ret;
}


struct image_buffer * LinuxCamera::fetch( bool lastOne )
{
    struct image_buffer * retBuffer = nullptr;

    if( !this->isOpen() ) log( "Unable to call fetch() as no device is open", warning );
    {
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
    }

    return retBuffer;
}


bool LinuxCamera::enumVideoModes()
{
    int offset = 0;
    bool ret = false;

    // clear the existing video structure
    this->m_modes.clear();

    // make sure fid is valid
    if( !this->isOpen() ) log( "Unable to call enumVideoModes() as device is NOT open", warning );

    else
    {
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
    }

    return ret;
}


bool LinuxCamera::enumControls()
{
    bool ret = false;

    // clear the existing control structure
    this->m_controls.clear();

    // make sure fid is valid
    if( !this->isOpen() ) log( "Unable to call enumControls() as device is NOT open", warning );

    else
    {
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
    }

    return ret;
}


std::string LinuxCamera::cntrlTypeToString( int type )
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
