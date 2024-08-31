#include <cstring>
#include <string>
#include <iostream>
#include <vector>

// using ioctl for low level device enumeration and control
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <fcntl.h>

#include "v4l2camera.h"

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

V4l2Camera::~V4l2Camera()
{
    // close the device before we disappear - if m_fid is set then the device is likely open
    if( this->m_fid > -1 ) ::close(this->m_fid);
}

std::string V4l2Camera::getFidName()
{
    return this->m_fidName;
}

std::string V4l2Camera::getUserName()
{
    return this->m_userName;
}

bool V4l2Camera::canOpen()
{
    int fd = ::open(this->m_fidName.c_str(), O_RDWR);
    ::close(fd);

    return( fd > -1 );
}

bool V4l2Camera::canFetch()
{
    return ( this->m_capabilities & V4L2_CAP_VIDEO_CAPTURE );
}

bool V4l2Camera::canRead()
{
    return ( this->m_capabilities & V4L2_CAP_READWRITE );
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
        std::string msg = "[" + this->getTagStr(tag) + "] " + this->m_fidName +  " : " + out;

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

int V4l2Camera::setValue( int id, int newVal )
{
    struct v4l2_control outQuery;
    int ret = -1;

    // this is the actual control ID, get the value to set
    if( -1 == this->m_fid ) log( "Unable to call setValue() as device is NOT open", warning );

    else
    {
        memset( &outQuery, 0, sizeof(struct v4l2_control));
        outQuery.id = id;
        outQuery.value = newVal;
        ret = ioctl(this->m_fid, VIDIOC_S_CTRL, &outQuery );

        if( -1 == ret ) log( "ioctl(VIDIOC_S_CTRL) [" + std::to_string(id) + "] failed :  " + strerror(errno), info );
        else log( "ioctl(VIDIOC_S_CTRL) [" + std::to_string(id) + "] = " + std::to_string(newVal), info );
    }

    return ret;
}

int V4l2Camera::getValue( int id )
{
    struct v4l2_control outQuery;

    int ret = -1;

    // this is the actual control ID, get the value to set
    if( -1 == this->m_fid ) log( "Unable to call getValue() as device is NOT open", warning );

    else
    {
        memset( &outQuery, 0, sizeof(struct v4l2_control));
        outQuery.id = id;
        if( -1 == ioctl(m_fid, VIDIOC_G_CTRL, &outQuery ) ) log( "ioctl(VIDIOC_G_CTRL) [" + std::to_string(id) + "] failed :  " + strerror(errno), info );
        else
        {
            ret = outQuery.value;
            log( "ioctl(VIDIOC_G_CTRL) [" + std::to_string(id) +"] = " + std::to_string(ret), info );
        }
    }

    return ret;
}

bool V4l2Camera::open()
{
    bool ret = false;

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

    return ret;
}

bool V4l2Camera::isOpen()
{
    return( this->m_fid > -1 );
}

bool V4l2Camera::enumCapabilities()
{
    struct v4l2_capability tmpV;

    bool ret = false;

    if( -1 == this->m_fid ) log( "Unable to call getCaps() as device is NOT open", warning );

    else
    {
        if( -1 == ioctl(this->m_fid, VIDIOC_QUERYCAP, &tmpV) ) log( "ioctl(VIDIOC_QUERYCAP) failed :  " + std::string(strerror(errno)), error );

        else
        {
            // grab the device name
            this->m_userName = (char *)(tmpV.card);
            this->m_capabilities = tmpV.capabilities;
            ret = true;
            log( "ioctl(VIDIOC_QUERYCAP) " + this->m_fidName + " success, vector = " + std::to_string(this->m_capabilities), info );
        }
    }

    return ret;
}

bool V4l2Camera::checkCapabilities( unsigned int val )
{
        return ( this->m_capabilities & val );
}

void V4l2Camera::close()
{
    enum v4l2_buf_type type;

    // disable streaming mode so the buffers are no longer being filled
    switch( this->m_bufferMode )
    {
        case notset:
        case readMode:
            break;

        case userPtrMode:
        case mMapMode:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if( -1 == ioctl( this->m_fid, VIDIOC_STREAMOFF, &type) ) log( "ioctl(VIDIOC_STREAMOFF) failed : " + std::string(strerror(errno)), error );
            else log( "VIDIOC_STREAMOFF success (streaming off) for " + this->m_fidName, info );
            break;
    }

    ::close(this->m_fid);
    this->m_fid = -1;

    log( this->m_fidName + " closed", info );
}

bool V4l2Camera::setFrameFormat( struct video_mode vm )
{
    struct v4l2_format fmt;

    bool ret = false;
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

    if( -1 == this->m_fid ) log( "Unable to call init() as device is NOT open", info );
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
                break;

            case notset:
                break;
        }
    }

    return ret;
}

struct image_buffer * V4l2Camera::fetch( bool lastOne )
{
    struct image_buffer * retBuffer = nullptr;

    if( -1 == this->m_fid ) log( "Unable to call fetch() as no device is open", warning );
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

                    log( "ioctl(VIDIOC_DQBUF) success, DeQ'd "
                            + std::to_string(((float)buf.bytesused/1024.)) + " kB of "
                            + std::to_string(((float)this->m_frameBuffer->length/1024.)) +  "kB allocated", info );

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
                        else log( "ioctl(VIDIOC_QBUF) success, Re-Q'd 1 buffer", info );
                    }
                }
                break;

            case mMapMode:
                break;

            case notset:
                break;
        }
    }

    return retBuffer;
}

struct video_mode V4l2Camera::getOneVM( int index )
{
    return this->m_modes[index];
}

bool V4l2Camera::enumVideoModes()
{
    int offset = 0;
    struct v4l2_fmtdesc tmpF;
    struct v4l2_frmsizeenum tmpS;

    memset( &tmpF, 0, sizeof(tmpF) );
    memset( &tmpS, 0, sizeof(tmpS) );

    bool ret = false;

    // clear the existing video structure
    this->m_modes.clear();

    // make sure fid is valid
    if( -1 == this->m_fid ) log( "Unable to call enumVideoModes() as device is NOT open", warning );

    else
    {
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

struct user_control V4l2Camera::getOneCntrl( int index )
{
    return this->m_controls[index];
}

bool V4l2Camera::enumControls()
{
    bool ret = false;

    // make sure fid is valid
    if( -1 == this->m_fid ) log( "Unable to call enumControls() as device is NOT open", warning );

    else
    {
        log( "ioctl(VIDIOC_QUERY_EXT_CTRL) for : " + this->m_userName, info );

        // clear the existing control structure
        this->m_controls.clear();

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

std::string V4l2Camera::cntrlTypeToString( int type )
{
    std::string ret = "unknown";

    switch( type )
    {
        case V4L2_CTRL_TYPE_INTEGER:
            ret = "Integer";
            break;
        case V4L2_CTRL_TYPE_BOOLEAN:
            ret = "Boolean";
            break;
        case V4L2_CTRL_TYPE_MENU:
            ret = "Menu";
            break;
        case V4L2_CTRL_TYPE_INTEGER_MENU:
            ret = "Integer Menu";
            break;
        case V4L2_CTRL_TYPE_BITMASK:
            ret = "Bitmask";
            break;
        case V4L2_CTRL_TYPE_BUTTON:
            ret = "Button";
            break;
        case V4L2_CTRL_TYPE_INTEGER64:
            ret = "Integer 64";
            break;
        case V4L2_CTRL_TYPE_STRING:
            ret = "String";
            break;
        case V4L2_CTRL_TYPE_CTRL_CLASS:
            ret = "Control Class";
            break;
        case V4L2_CTRL_TYPE_U8:
            ret = "Integer U8";
            break;
        case V4L2_CTRL_TYPE_U16:
            ret = "Integer U16";
            break;
        case V4L2_CTRL_TYPE_U32:
            ret = "Integer U32";
            break;
        default:
            break;
    }

    return ret;
}
