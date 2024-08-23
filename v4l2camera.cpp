#include "v4l2camera.h"

#include <cstring>
#include <string>
#include <iostream>
#include <vector>

// using ioctl for low level device enumeration and control
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <fcntl.h>

v4l2Camera::v4l2Camera( std::string name )
{
    this->fidName = name;
    this->devName = "";
    this->myCaps = 0;
    this->fid = -1;
    this->myFetchMode = notset;

    // initiallize the return buffers
    this->frameBuffer = nullptr;
}

std::string v4l2Camera::getFidName()
{
    return this->fidName;
}

std::string v4l2Camera::getDevName()
{
    return this->devName;
}

bool v4l2Camera::canOpen()
{
    int fd = ::open(fidName.c_str(), O_RDWR);
    ::close(fd);

    return( fd > -1 );
}

bool v4l2Camera::canFetch()
{
    return ( this->myCaps & V4L2_CAP_VIDEO_CAPTURE );
}

bool v4l2Camera::canFetch_Read()
{
    return ( this->myCaps & V4L2_CAP_READWRITE );
}

int v4l2Camera::setValue( int id, int newVal )
{
    struct v4l2_control outQuery;
    int ret = -1;

    // this is the actual control ID, get the value to set
    if( this->fid > -1 )
    {
        memset( &outQuery, 0, sizeof(struct v4l2_control));
        outQuery.id = id;
        outQuery.value = newVal;
        ret = ioctl(fid, VIDIOC_S_CTRL, &outQuery );
        std::cout << "setValue(" << id << ") to " << newVal << std::endl;

    } else std::cout << "Unable to call setValue() as no device is open" << std::endl;

    return ret;
}

int v4l2Camera::getValue( int id )
{
    struct v4l2_control outQuery;

    int ret = -1;

    // this is the actual control ID, get the value to set
    if( this->fid > -1 )
    {
        memset( &outQuery, 0, sizeof(struct v4l2_control));
        outQuery.id = id;
        if( ioctl(fid, VIDIOC_G_CTRL, &outQuery ) != -1 ) ret = outQuery.value;
        std::cout << "getValue(" << id << ") is " << ret << std::endl;

    } else std::cout << "Unable to call getValue() as no device is open" << std::endl;

    return ret;
}

bool v4l2Camera::open()
{
    bool ret = false;

    this->fid = ::open(fidName.c_str(), O_RDWR);
    if( this->fid > -1 )
    {
        // set the fetch mode to USERPtr as a default, UNTIL WE SUPPORT MMAP
        this->myFetchMode = userPtrMode;

        // indicate we are open ok
        ret = true;
        std::cout << "Device " << this->fidName << " opened successfully with FID = " << this->fid << std::endl;
    }
    else std::cout << "Device " << this->fidName << " failed to open : " << strerror(errno) << std::endl;

    return ret;
}

bool v4l2Camera::isOpen()
{
    return( this->fid > -1 );
}

bool v4l2Camera::getCaps()
{
    struct v4l2_capability tmpV;

    bool ret = false;

    if( this->fid > -1 )
    {
        if( ioctl(fid, VIDIOC_QUERYCAP, &tmpV) > -1 )
        {
            // grab the device name
            this->devName = (char *)(tmpV.card);
            this->myCaps = tmpV.capabilities;
            ret = true;
            std::cout << "getCaps( " << this->devName << " ) returned vector = " << this->myCaps << std::endl;
        }
    } else std::cout << "Unable to call getCaps() as no device is open" << std::endl;

    return ret;
}

void v4l2Camera::close()
{
    enum v4l2_buf_type type;

    // disable streaming mode so the buffers are no longer being filled
    switch( this->myFetchMode )
    {
        case notset:
        case readMode:
            break;

        case userPtrMode:
        case mMapMode:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if( ioctl( this->fid, VIDIOC_STREAMOFF, &type) > -1 ) std::cout << "VIDIOC_STREAMOFF success (streaming off) for " << this->fidName << std::endl;
            else std::cout << " ioctl(VIDIOC_STREAMOFF) failed : " << strerror(errno) << std::endl;
            break;
    }

    ::close(this->fid);
    this->fid = -1;

    std::cout << this->fidName << " closed" << std::endl;
}

bool v4l2Camera:: setFrameFormat( struct videoMode vm )
{
    struct v4l2_format fmt;

    bool ret = false;
    if( this->fid > -1 )
    {
        fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.pixelformat = vm.pixelFormat;
        fmt.fmt.pix.width       = vm.width;
        fmt.fmt.pix.height      = vm.height;

        if (ioctl(this->fid, VIDIOC_S_FMT, &fmt) > -1)
        {
            std::cout << "VIDIOC_S_FMT success (frame format) as : " << vm.pixelStr << " at [" << vm.width << " x " << vm.height << "]" << std::endl;
            this->curMode = vm;
            ret = true;

        } else std::cout << "ioctl(VIDIOC_S_FMT) failed : " << strerror(errno) << std::endl;

    } else std::cout << "Unable to call setFrameFormat() as no device is open" << std::endl;

    return ret;
}


bool v4l2Camera::init( enum fetchMode newMode )
{
    bool ret = false;

    if( this->fid > -1 )
    {
        this->myFetchMode = newMode;

        switch( this->myFetchMode )
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

                if( ioctl(this->fid, VIDIOC_REQBUFS, &req) > -1 )
                {
                    std::cout << "VIDIOC_REQBUFS success for " << this->fidName << std::endl;

                    // queuing up this->numBuffers fetch buffers
                    this->frameBuffer = new struct imageBuffer;
                    this->frameBuffer->length =  this->curMode.imgSize;
                    this->frameBuffer->buffer = new unsigned char[this->curMode.imgSize];

                    // queue up the buffer
                    struct v4l2_buffer buf;

                    memset(&buf, 0, sizeof(struct v4l2_buffer));
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_USERPTR;
                    buf.index = 0;
                    buf.m.userptr = (unsigned long)(this->frameBuffer->buffer);
                    buf.length = this->frameBuffer->length;

                    if( ioctl(this->fid, VIDIOC_QBUF, &buf) == -1 )
                    {
                        std::cout << "ioctl VIDIOC_QBUF failed : " << strerror(errno) << std::endl;
                        return false;
                    } else std::cout << "Q'd 1 buffer, for " << this->fidName << std::endl;

                    // turn streaming on
                    enum v4l2_buf_type type;
                    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    if( ioctl(this->fid, VIDIOC_STREAMON, &type) > -1 )
                    {
                        std::cout << "VIDIOC_STREAMON activated for " << this->fidName << std::endl;
                        ret = true;

                    } else std::cout << "ioctl VIDIOC_STREAMON failed : " << strerror(errno) << std::endl;
                } else {
                    std::cout << "ioctl VIDIOC_REQBUF failed : " << strerror(errno) << std::endl;
                }

                break;

            case mMapMode:
                break;

            case notset:
                break;
        }

    } else std::cout << "Unable to call init() as no device is open" << std::endl;

    return ret;
}

struct imageBuffer * v4l2Camera::fetch( bool lastOne )
{
    struct imageBuffer * retBuffer = nullptr;

    if( this->fid > -1 )
    {
        switch( this->myFetchMode )
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

                if( ioctl(this->fid, VIDIOC_DQBUF, &buf) > -1)
                {
                    retBuffer = new struct imageBuffer;
                    // this should have de-queued into the previous buffer we allocated
                    retBuffer->buffer = this->frameBuffer->buffer;
                    retBuffer->length = buf.bytesused;

                    std::cout << "Buffer "
                              << ((float)buf.bytesused/1024.) << " kB of "
                              << ((float)this->frameBuffer->length/1024.) <<" kB allocated : DeQ'd from " << this->fidName << std::endl;

                } else std::cout << "ioctl( VIDIOC_DQBUF ) failed : " << strerror(errno) << std::endl;

                // only re-queue if we are going to be getting more
                if( !lastOne )
                {
                    std::cout << "Re-queueing buffer for " << this->fidName << std::endl;

                    // aloocating for this->numBuffers fetch buffers
                    this->frameBuffer = new struct imageBuffer;
                    this->frameBuffer->length =  this->curMode.imgSize;
                    this->frameBuffer->buffer = new unsigned char[this->curMode.imgSize];

                    // queue up the first buffer
                    struct v4l2_buffer buf;

                    memset(&buf, 0, sizeof(struct v4l2_buffer));
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_USERPTR;
                    buf.index = 0;
                    buf.m.userptr = (unsigned long)(this->frameBuffer->buffer);
                    buf.length = this->frameBuffer->length;

                    if( ioctl(this->fid, VIDIOC_QBUF, &buf) == -1 ) std::cout << "ioctl( VIDIOC_QBUF ) failed : " << strerror(errno) << std::endl;
                    else std::cout << "Re-Q'd 1 buffer, for " << this->fidName << std::endl;

                }
                break;

            case mMapMode:
                break;

            case notset:
                break;
        }

    } else std::cout << "Unable to call setValue() as no device is open" << std::endl;

    return retBuffer;
}

bool v4l2Camera::enumVideoModes()
{
    int offset = 0;
    struct v4l2_fmtdesc tmpF;
    struct v4l2_frmsizeenum tmpS;

    memset( &tmpF, 0, sizeof(tmpF) );
    memset( &tmpS, 0, sizeof(tmpS) );

    bool ret = false;

    // clear the existing video structure
    myModes.clear();

    // make sure fid is valid
    if( this->fid > -1 )
    {
        std::cout << "Querying Pixel Formats from [" << this->devName << "]" << std::endl;

        // walk the list of pixel formats, for each format, walk the list of video modes (sizes)
        tmpF.index = 0;
        tmpF.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        while( ioctl(fid, VIDIOC_ENUM_FMT, &tmpF ) > -1 )
        {
            std::cout << "Querying video modes for format [" << tmpF.description << "]" << std::endl;

            // walk tje frame sizes for this pixel format
            tmpS.index = 0;
            tmpS.pixel_format = tmpF.pixelformat;
            while( ioctl( fid, VIDIOC_ENUM_FRAMESIZES, &tmpS ) > -1 )
            {
                std::cout << "Found : " << tmpS.discrete.width << " x " << tmpS.discrete.height << std::endl;

                // save all the info
                myModes[offset].pixelFormat = tmpF.pixelformat;
                myModes[offset].pixelStr = (char*)(tmpF.description);
                myModes[offset].imgSize = tmpS.discrete.width * 4 * tmpS.discrete.height;
                myModes[offset].width = tmpS.discrete.width;
                myModes[offset++].height = tmpS.discrete.height;

                tmpS.index++;
            }

            tmpF.index++;
        }
        ret = true;

    } else std::cout << "Unable to call enumVideoModes() as no device is open" << std::endl;

    return ret;
}

bool v4l2Camera::enumControls()
{
    bool ret = false;

    // make sure fid is valid
    if( this->fid > -1 )
    {
        std::cout << "Querying User Controls from [" << this->devName << "]" << std::endl;

        // clear the existing control structure
        myControls.clear();

        // walk all the private extended controls starting at V4L2_CID_PRIVATE_BASE
        // call should fail when there are no more private controls
        struct v4l2_query_ext_ctrl query_ext_ctrl;

        memset(&query_ext_ctrl, 0, sizeof(query_ext_ctrl));

        query_ext_ctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
        while (0 == ioctl(fid, VIDIOC_QUERY_EXT_CTRL, &query_ext_ctrl))
        {
            if (!(query_ext_ctrl.flags & V4L2_CTRL_FLAG_DISABLED))
            {
                if( query_ext_ctrl.type != V4L2_CTRL_TYPE_CTRL_CLASS )
                {
                    std::cout <<"Found : " << query_ext_ctrl.name << std::endl;

                    myControls[query_ext_ctrl.id].name = (char*)(query_ext_ctrl.name);
                    myControls[query_ext_ctrl.id].value = query_ext_ctrl.default_value;
                    myControls[query_ext_ctrl.id].type = query_ext_ctrl.type;
                    myControls[query_ext_ctrl.id].typeStr = setTypeStr(query_ext_ctrl.type);
                    myControls[query_ext_ctrl.id].min = query_ext_ctrl.minimum;
                    myControls[query_ext_ctrl.id].max = query_ext_ctrl.maximum;
                    myControls[query_ext_ctrl.id].step = query_ext_ctrl.step;
                } else {
                    std::cout <<"Skipping : " << query_ext_ctrl.name << std::endl;
                }
            }

            query_ext_ctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }
        ret = true;

    } else std::cout << "Unable to call enumControls() as no device is open" << std::endl;

    return ret;
}

std::string v4l2Camera::setTypeStr( int type )
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
