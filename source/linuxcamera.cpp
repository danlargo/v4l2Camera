#include <cstring>
#include <iostream>

// using ioctl for low level device enumeration and control
#include <sys/ioctl.h>
#include <linux/videodev2.h>

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

    m_cameraType = "generic Linux UVC";

    m_healthCounter = 0;

    memset(&buf, 0, NUM_QBUF* sizeof(struct v4l2_buffer));
}


LinuxCamera::~LinuxCamera()
{
    // close the device before we disappear - if m_fid is set then the device is likely open
    if( m_fid > -1 ) ::close(m_fid);

    // free the buffers if they have been allocated
    for( int i=0;i<NUM_QBUF;i++ )
    {
        if( buf[i].m.userptr > 0 ) delete (unsigned char *)buf[i].m.userptr;
    }

}


std::vector<LinuxCamera *> LinuxCamera::discoverCameras( v4l2cam_logging_mode logMode, bool streamingOnly )
{
    std::vector<LinuxCamera *> camList;
    int count = 0;

    std::vector<std::string> devList = LinuxCamera::buildCamList();

    for( const auto &x : devList )
    {
        bool keep = false;

        // create the camera object
        LinuxCamera * tmpC = new LinuxCamera(x);
        tmpC->setLogMode( logMode );

        std::string nam = x;

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
                        tmpC->enumMetadataModes();

                        if( tmpC->m_capabilities > 0 )
                        {
                            if( !streamingOnly || (streamingOnly && (tmpC->m_capabilities & V4L2_CAP_STREAMING) && (tmpC->m_modes.size() > 0) ) )
                            {
                                // save the camera for display later
                                keep = true;
                                camList.push_back(tmpC);
                            }
                        }
                    }
                }
                // close the camera
                tmpC->close();
            }
        } else tmpC->log( nam + " : failed to create V4l2Camera object for device", info );

        if( !keep ) delete tmpC;
    }

    return camList;
}


std::vector<std::string> LinuxCamera::buildCamList()
{
    std::vector<std::string> ret;
    int maxCams = 64;

    for( int i=0;i<maxCams;i++ ) ret.push_back( "/dev/video" + std::to_string(i) );

    return ret;
}

//
// Basic Access routines
//
bool LinuxCamera::open()
{
    bool ret = false;

    m_fid = ::open(m_devName.c_str(), O_RDWR);

    if( -1 == m_fid ) m_healthCounter = s_healthCountLimit;
    else
    {
        // set the fetch mode to USERPtr as a default
        m_bufferMode = userPtrMode;

        // indicate we are open ok
        ret = true;
        m_healthCounter = 0;

        log( m_devName + " opened", info );
    }

    return ret;
}

bool LinuxCamera::isOpen()
{
    return( m_fid > -1 );
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
            // this will fail if STREAMON has never been executed, that is ok
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            ioctl( m_fid, VIDIOC_STREAMOFF, &type);
            break;
    }

    ::close(m_fid);
    m_fid = -1;

    log( m_devName + " closed", info );

}

//
// Setup for fetching frames
//
bool LinuxCamera::init( enum v4l2cam_fetch_mode newMode )
{
    bool ret = false;

    if( !isOpen() ) log( "Unable to call init() as device is NOT open", info );
    else 
    {
        m_bufferMode = newMode;

        switch( m_bufferMode )
        {
            case readMode:
                // do nothing, it is already set to UserPtr mode
                ret = true;
                break;

            case userPtrMode:
                struct v4l2_requestbuffers req;

                memset(&req,0,sizeof(struct v4l2_requestbuffers));

                req.count  = NUM_QBUF;
                req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                req.memory = V4L2_MEMORY_USERPTR;

                if( -1 == ioctl(m_fid, VIDIOC_REQBUFS, &req) ) 
                {
                    log( "ioctl(VIDIOC_REQBUF) failed : " + std::string(strerror(errno)), error );
                    m_healthCounter++;
                }
                else
                {
                    // queuing up this->numBuffers fetch buffers
                    //m_frameBuffer = new struct v4l2cam_image_buffer;
                    //m_frameBuffer->length =  m_currentMode.size;
                    //m_frameBuffer->buffer = new unsigned char[this->m_currentMode.size];

                    // queue up the buffer
                    //struct v4l2_buffer buf;

                    memset(&buf, 0, NUM_QBUF * sizeof(struct v4l2_buffer));

                    // queue up all the buffers
                    for( int i=0;i<NUM_QBUF;i++ )
                    {
                        buf[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf[i].memory = V4L2_MEMORY_USERPTR;
                        buf[i].index = i;
                        //buf.m.userptr = (unsigned long)(m_frameBuffer->buffer);
                        buf[i].m.userptr = (unsigned long)(new unsigned char[this->m_currentMode.size]);
                        //buf.length = m_frameBuffer->length;
                        buf[i].length = m_currentMode.size;

                        if( -1 == ioctl(m_fid, VIDIOC_QBUF, &(buf[i]) ) )
                        {
                            log( "ioctl(VIDIOC_QBUF) failed : " + std::string(strerror(errno)), error );
                            m_healthCounter++;
                        }
                        else
                        {
                            // turn streaming on
                            enum v4l2_buf_type type;
                            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                            if( -1 == ioctl(m_fid, VIDIOC_STREAMON, &type) ) 
                            {
                                log( "ioctl(VIDIOC_STREAMON) failed : " + std::string(strerror(errno)), error );
                                m_healthCounter++;
                            }
                            else
                            {
                                ret = true;
                                m_healthCounter = 0;
                            }
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


//
// Data Retreiaval routines
//
struct v4l2cam_image_buffer * LinuxCamera::fetch( bool lastOne )
{
    struct v4l2cam_image_buffer * retBuffer = nullptr;

    if( !isOpen() ) log( "Unable to call fetch() as no device is open", warning );
    else 
    {
        switch( m_bufferMode )
        {
            case readMode:
                // do nothing
                break;

            case userPtrMode:
                // dequeue one frame
                struct v4l2_buffer tmp_buf;
                memset(&tmp_buf, 0, sizeof(struct v4l2_buffer));

                tmp_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                tmp_buf.memory = V4L2_MEMORY_USERPTR;

                if( -1 == ioctl(m_fid, VIDIOC_DQBUF, &tmp_buf) ) 
                {
                    log( "ioctl(VIDIOC_DQBUF) failed : " + std::string(strerror(errno)), error );
                    m_healthCounter++;
                }
                else
                {
                    retBuffer = new struct v4l2cam_image_buffer;
                    // this should have de-queued into the previous buffer we allocated
                    //retBuffer->buffer = m_frameBuffer->buffer;
                    //retBuffer->buffer = (unsigned char*)buf.m.userptr;
                    retBuffer->buffer = new unsigned char[tmp_buf.bytesused];
                    retBuffer->length = tmp_buf.bytesused;
                    memcpy( retBuffer->buffer, (unsigned char*)tmp_buf.m.userptr, tmp_buf.bytesused );

                    // only re-queue if we are going to be getting more
                    if( !lastOne )
                    {
                        // aloocating for this->numBuffers fetch buffers
                        //m_frameBuffer = new struct v4l2cam_image_buffer;
                        //m_frameBuffer->length =  m_currentMode.size;
                        //m_frameBuffer->buffer = new unsigned char[this->m_currentMode.size];

                        // queue up the first buffer
                        //struct v4l2_buffer buf;

                        //memset(&buf, 0, sizeof(struct v4l2_buffer));
                        //buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        //buf.memory = V4L2_MEMORY_USERPTR;
                        //buf.index = 0;
                        //buf.m.userptr = (unsigned long)(this->m_frameBuffer->buffer);
                        //buf.length = m_frameBuffer->length;

                        // just re-queue the one that was already there

                        if( -1 == ioctl(m_fid, VIDIOC_QBUF, &tmp_buf) ) 
                        {
                            log( "ioctl(VIDIOC_QBUF) failed : " + std::string(strerror(errno) ), error );
                            m_healthCounter++;

                        }else m_healthCounter = 0;
                    } else m_healthCounter = 0;
                }
                break;

            case mMapMode:
            case notset:
                break;
        }
    }

    return retBuffer;
} 

//
// Device Capability routines
//
bool LinuxCamera::canFetch()
{
    return ( m_capabilities & V4L2_CAP_VIDEO_CAPTURE );
}


std::string LinuxCamera::getDevName()
{
    return m_devName;
}


bool LinuxCamera::canRead()
{
    return ( m_capabilities & V4L2_CAP_READWRITE );
}

bool LinuxCamera::hasMetaData()
{
    return ( m_capabilities & V4L2_CAP_META_CAPTURE );
}


//
// Camera Control routines
//
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
        if( !open() )
        {
            log( "Unable to openOnDemand for setValue() : " + std::string(strerror(errno)), error );
            return -1;
        }
        closeOnExit = true;
    }

    memset( &outQuery, 0, sizeof(struct v4l2_control));
    outQuery.id = id;
    outQuery.value = newVal;
    ret = ioctl(m_fid, VIDIOC_S_CTRL, &outQuery );

    if( -1 == ret ) 
    {
        log( "ioctl(VIDIOC_S_CTRL) [" + std::to_string(id) + "] failed :  " + strerror(errno), info );
        m_healthCounter++;
    }
    else m_healthCounter = 0;

    if( closeOnExit )
    {
        ::close( m_fid );
        m_fid = -1;
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
        if( !open() )
        {
            log( "Unable to openOnDemand for getValue() : " + std::string(strerror(errno)), error );
            return -1;
        }
        closeOnExit = true;
    }

    // get the actual value
    memset( &outQuery, 0, sizeof(struct v4l2_control));
    outQuery.id = id;
    if( -1 == ioctl(m_fid, VIDIOC_G_CTRL, &outQuery ) ) 
    {
        log( "ioctl(VIDIOC_G_CTRL) [" + std::to_string(id) + "] failed :  " + strerror(errno), info );
        m_healthCounter++;
    }
    else
    {
        ret = outQuery.value;
        m_healthCounter = 0;
    }

    // close it if we opened on demand
    if( closeOnExit )
    {
        ::close(m_fid);
        m_fid = -1;
    }

    return ret;
}


bool LinuxCamera::enumCapabilities()
{
    bool ret = false;

    if( !isOpen() ) log( "Unable to call getCaps() as device is NOT open", warning );
    else
    {
        struct v4l2_capability tmpV;

        if( -1 == ioctl(m_fid, VIDIOC_QUERYCAP, &tmpV) ) 
        {
            log( "ioctl(VIDIOC_QUERYCAP) failed :  " + std::string(strerror(errno)), error );
            m_healthCounter++;
        }
        else
        {
            // grab the device name
            m_userName = (char *)(tmpV.card);
            // truncate the name if it is duplicated
            int colon = m_userName.find(":");
            if(  colon > -1 ) m_userName = m_userName.substr(0,m_userName.find(":"));

            // save the capabilities vector
            m_capabilities = tmpV.capabilities;
            ret = true;
            m_healthCounter = 0;
        }
    }

    return ret;
}

std::vector<std::string> LinuxCamera::capabilitiesToStr()
{
    std::vector<std::string> ret = {};

    if( 0 == m_capabilities ) return ret;

    // if( m_capabilities & V4L2_CAP_DEVICE_CAPS ) ret += "device-caps ";

    if( (m_capabilities & V4L2_CAP_STREAMING) && (m_modes.size() > 0) ) ret.push_back("can stream");
    if( (m_capabilities & V4L2_CAP_EXT_PIX_FORMAT) && (m_modes.size() > 0) ) ret.push_back("can query pixel formats");
    
    if( (m_capabilities & V4L2_CAP_META_CAPTURE) && (m_metamode > 0) ) ret.push_back("can read metadata");
    if( (m_capabilities & V4L2_CAP_META_OUTPUT ) && (m_metamode > 0) ) ret.push_back("can write metadata");

    if( m_capabilities & V4L2_CAP_READWRITE ) ret.push_back("can read/write");
    if( m_capabilities & V4L2_CAP_ASYNCIO ) ret.push_back("supports async IO");

    if( (m_capabilities & V4L2_CAP_VIDEO_CAPTURE) && (m_modes.size() > 0) ) ret.push_back("supports single-planar video capture");
    if( m_capabilities & V4L2_CAP_VIDEO_OUTPUT ) ret.push_back("video-output");
    if( m_capabilities & V4L2_CAP_VIDEO_OVERLAY ) ret.push_back("video-overlay");
    if( m_capabilities & V4L2_CAP_TIMEPERFRAME ) ret.push_back("timerperframe");

    if( m_capabilities & V4L2_CAP_VBI_CAPTURE ) ret.push_back("vbi-capture");
    if( m_capabilities & V4L2_CAP_VBI_OUTPUT ) ret.push_back("vbi-output");
    if( m_capabilities & V4L2_CAP_SLICED_VBI_CAPTURE ) ret.push_back("slided-vbi-capture");
    if( m_capabilities & V4L2_CAP_SLICED_VBI_OUTPUT ) ret.push_back("slided-vbi-output");

    if( m_capabilities & V4L2_CAP_RDS_CAPTURE ) ret.push_back("rds-capture");
    if( m_capabilities & V4L2_CAP_RDS_OUTPUT ) ret.push_back("rds-output");
    if( m_capabilities & V4L2_CAP_SDR_CAPTURE ) ret.push_back("sdr-capture");

    if( m_capabilities & V4L2_CAP_VIDEO_OUTPUT_OVERLAY ) ret.push_back("video-output-overlay");

    if( m_capabilities & V4L2_CAP_VIDEO_M2M ) ret.push_back("video-m2m");
    if( m_capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE ) ret.push_back("supports multi-planar video capture");
    if( m_capabilities & V4L2_CAP_VIDEO_OUTPUT_MPLANE ) ret.push_back("video-output-mplane");
    if( m_capabilities & V4L2_CAP_VIDEO_M2M_MPLANE ) ret.push_back("video-m2m-mplane");

    if( m_capabilities & V4L2_CAP_HW_FREQ_SEEK ) ret.push_back("hw-freq-seek");
    if( m_capabilities & V4L2_CAP_TUNER ) ret.push_back("tuner");
    if( m_capabilities & V4L2_CAP_AUDIO ) ret.push_back("audio");
    if( m_capabilities & V4L2_CAP_RADIO ) ret.push_back("radio");
    if( m_capabilities & V4L2_CAP_MODULATOR ) ret.push_back("modulator");

    return ret;
}


struct v4l2cam_video_mode * LinuxCamera::getFrameFormat()
{
    struct v4l2cam_video_mode * ret = nullptr;

    if( !isOpen() ) log( "Unable to call getFrameFormat() as no device is open", warning );
    else 
    {
        struct v4l2_format fmt;

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if( -1 == ioctl(m_fid, VIDIOC_G_FMT, &fmt) ) 
        {
            log( "ioctl(VIDIOC_G_FMT) failed : " + std::string(strerror(errno)), error );
            m_healthCounter++;
        }
        else
        {
            ret = new struct v4l2cam_video_mode;
            ret->fourcc = fmt.fmt.pix.pixelformat;
            ret->width = fmt.fmt.pix.width;
            ret->height = fmt.fmt.pix.height;
            ret->size = fmt.fmt.pix.sizeimage;
            char fStr[256];
            V4l2Camera::fourcc_int_to_charArray(ret->fourcc, fStr);
            ret->format_str = fStr;
            m_healthCounter = 0;

            // set the current mode, from this discovered mode
            m_currentMode = *ret;
        }
    }

    return ret;
}


int LinuxCamera::getFrameRate()
{
    int ret = -1;

    if( !isOpen() ) log( "Unable to call getFrameRate() as no device is open", warning );
    else 
    {
        struct v4l2_streamparm streamparm;
        memset(&streamparm, 0, sizeof(streamparm));
        streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if( -1 == ioctl( m_fid, VIDIOC_G_PARM, &streamparm) )
        {
            log( "ioctl(VIDIOC_G_PARM - FrameRate) failed : " + std::string(strerror(errno)), error );
            m_healthCounter++;
        }
        else
        {
            ret = streamparm.parm.capture.timeperframe.denominator;
            m_healthCounter = 0;
        }
    }

    return ret;
}

bool LinuxCamera::setFrameFormat( std::string mode, int width, int height )
{
    // included here to allow it to be overridden by inherited classes
    return V4l2Camera::setFrameFormat( mode, width, height );
}

bool LinuxCamera::setFrameFormat( struct v4l2cam_video_mode vm, int fps )
{
    bool ret = false;

    struct v4l2_format fmt;

    if( -1 == m_fid ) log( "Unable to call setFrameFormat() as device is NOT open", warning );
    else 
    {
        fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.pixelformat = vm.fourcc;
        fmt.fmt.pix.width       = vm.width;
        fmt.fmt.pix.height      = vm.height;

        if( -1 == ioctl(m_fid, VIDIOC_S_FMT, &fmt) ) 
        {
            log( "ioctl(VIDIOC_S_FMT) failed : " + std::string(strerror(errno)), error );
            m_healthCounter++;
        }
        else
        {
            m_currentMode = vm;
            ret = true;
            m_healthCounter = 0;
        }

        // now set the frame rate, if  requested
        if( fps > 0 )
        {
            struct v4l2_streamparm streamparm;
            memset(&streamparm, 0, sizeof(streamparm));
            streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if( -1 == ioctl( m_fid, VIDIOC_G_PARM, &streamparm) )
            {
                log( "ioctl(VIDIOC_G_PARM - FrameRate) failed : " + std::string(strerror(errno)), error );
                m_healthCounter++;

            } else {
                streamparm.parm.capture.capturemode |= V4L2_CAP_TIMEPERFRAME;
                streamparm.parm.capture.timeperframe.numerator = 1;
                streamparm.parm.capture.timeperframe.denominator = fps;
                if( -1 == ioctl( m_fid,VIDIOC_S_PARM, &streamparm) !=0) 
                {
                    log( "ioctl(VIDIOC_S_PARM - FrameRate) failed : " + std::string(strerror(errno)), error );
                    m_healthCounter++;
                } else m_healthCounter = 0;
            }
        }
    }
    
    return ret;
}

bool LinuxCamera::setFrameRate( int fps )
{
    bool ret = false;

    if( -1 == m_fid ) log( "Unable to call setFrameFormat() as device is NOT open", warning );
    else 
    {
        // set the frame rate, if  requested
        if( fps > 0 )
        {
            struct v4l2_streamparm streamparm;
            memset(&streamparm, 0, sizeof(streamparm));
            streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if( -1 == ioctl( m_fid, VIDIOC_G_PARM, &streamparm) )
            {
                log( "ioctl(VIDIOC_G_PARM - FrameRate) failed : " + std::string(strerror(errno)), error );
                m_healthCounter++;

            } else {
                streamparm.parm.capture.capturemode |= V4L2_CAP_TIMEPERFRAME;
                streamparm.parm.capture.timeperframe.numerator = 1;
                streamparm.parm.capture.timeperframe.denominator = fps;
                if( -1 == ioctl( m_fid,VIDIOC_S_PARM, &streamparm) !=0) 
                {
                    log( "ioctl(VIDIOC_S_PARM - FrameRate) failed : " + std::string(strerror(errno)), error );
                    m_healthCounter++;
                } else  m_healthCounter = 0;
            }
        } else log( "Invalid frame rate requested : " + std::to_string(fps), error );

    }
    
    return ret;
}


struct v4l2cam_metadata_buffer * LinuxCamera::fetchMetaData()
{
    struct v4l2cam_metadata_buffer * retBuffer = nullptr;

    if( !isOpen() ) log( "Unable to call fetch() as no device is open", warning );
    else
    {
        // make sure the metadata is supported
        if( (0 == m_metamode) || (0 == m_metasize) ) log( "Metadata fetch is not supported on this device", warning );
        else
        {
            // Prepare the metadata format structure
            struct v4l2_format fmt;
            memset(&fmt, 0, sizeof(fmt));
            fmt.type = V4L2_BUF_TYPE_META_CAPTURE;
            fmt.fmt.meta.dataformat = m_metamode;
            
            // Set the format
            if( -1 == ioctl(m_fid, VIDIOC_S_FMT, &fmt) ) 
            {
                log( "ioctl(VIDIOC_S_FMT metadata) failed : " + std::string(strerror(errno)), error );
                m_healthCounter++;
                
            } else 
            {
                struct v4l2_requestbuffers req;
                memset(&req,0,sizeof(struct v4l2_requestbuffers));

                req.count  = 1;
                req.type   = V4L2_BUF_TYPE_META_CAPTURE;
                req.memory = V4L2_MEMORY_USERPTR;

                if( -1 == ioctl(m_fid, VIDIOC_REQBUFS, &req) ) 
                {
                    log( "ioctl(VIDIOC_REQBUF metadata) failed : " + std::string(strerror(errno)), error );
                    m_healthCounter++;

                } else
                {
                    // queuing up fetch buffer
                    struct v4l2cam_metadata_buffer * m_dataBuffer = new struct v4l2cam_metadata_buffer;
                    m_dataBuffer->length =  m_metasize;
                    m_dataBuffer->buffer = new unsigned char[m_metasize];

                    // queue up the buffer
                    struct v4l2_buffer buf;

                    memset(&buf, 0, sizeof(struct v4l2_buffer));
                    buf.type = V4L2_BUF_TYPE_META_CAPTURE;
                    buf.memory = V4L2_MEMORY_USERPTR;
                    buf.index = 0;
                    buf.m.userptr = (unsigned long)(m_dataBuffer->buffer);
                    buf.length = m_dataBuffer->length;

                    if( -1 == ioctl(m_fid, VIDIOC_QBUF, &buf) ) 
                    {
                        log( "ioctl(VIDIOC_QBUF metadata) failed : " + std::string(strerror(errno)), error );
                        m_healthCounter++;

                    } else
                    {
                        // Prepare the format structure
                        struct v4l2_format fmt;
                        memset(&fmt, 0, sizeof(fmt));
                        fmt.type = V4L2_BUF_TYPE_META_CAPTURE;
                        fmt.fmt.meta.dataformat = V4L2_META_FMT_UVC;
                        
                        // Set the format
                        if( -1 == ioctl(m_fid, VIDIOC_S_FMT, &fmt) ) 
                        {
                            log( "ioctl(VIDIOC_S_FMT metadata) failed : " + std::string(strerror(errno)), error );
                            m_healthCounter++;

                        } else 
                        {
                            // dequeue one frame
                            struct v4l2_buffer buf;
                            memset(&buf, 0, sizeof(struct v4l2_buffer));

                            buf.type = V4L2_BUF_TYPE_META_CAPTURE;
                            buf.memory = V4L2_MEMORY_USERPTR;

                            if( -1 == ioctl(m_fid, VIDIOC_DQBUF, &buf) ) 
                            {
                                log( "ioctl(VIDIOC_DQBUF metadata) failed : " + std::string(strerror(errno)), error );
                                m_healthCounter++;
                            }
                            else
                            {
                                retBuffer = new struct v4l2cam_metadata_buffer;
                                // this should have de-queued into the previous buffer we allocated
                                retBuffer->buffer = m_dataBuffer->buffer;
                                retBuffer->length = buf.bytesused;
                            }
                        }
                    }
                }
            }
        }
    }

    return retBuffer;
}

bool LinuxCamera::enumVideoModes()
{
    int offset = 0;
    bool ret = false;

    // clear the existing video structure
    m_modes.clear();

    // make sure fid is valid
    if( !isOpen() ) log( "Unable to call enumVideoModes() as device is NOT open", warning );
    else
    {
        struct v4l2_fmtdesc tmpF;
        struct v4l2_frmsizeenum tmpS;
        struct v4l2_frmivalenum tmpI;

        memset( &tmpF, 0, sizeof(tmpF) );
        memset( &tmpS, 0, sizeof(tmpS) );
        memset( &tmpI, 0, sizeof(tmpS) );

        // walk the list of pixel formats, for each format, walk the list of video modes (sizes)
        tmpF.index = 0;
        tmpF.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        while( -1 != ioctl(m_fid, VIDIOC_ENUM_FMT, &tmpF ) )
        {
            // walk the frame sizes for this pixel format
            tmpS.index = 0;
            tmpS.pixel_format = tmpF.pixelformat;
            while( -1 != ioctl( m_fid, VIDIOC_ENUM_FRAMESIZES, &tmpS ) )
            {
                // make sure the discrete type is set
                if((0 == tmpS.index) && (tmpS.type != V4L2_FRMSIZE_TYPE_DISCRETE) ) break;
                if( tmpS.type != V4L2_FRMSIZE_TYPE_DISCRETE ) continue;

                // save all the info
                struct v4l2cam_video_mode tmpVM;

                tmpVM.fourcc = tmpF.pixelformat;
                tmpVM.format_str = (char*)(tmpF.description);
                tmpVM.size = tmpS.discrete.width * 4 * tmpS.discrete.height;
                tmpVM.width = tmpS.discrete.width;
                tmpVM.height = tmpS.discrete.height;
                tmpVM.fps = {};

                // now find out the frame intervals for this size
                tmpI.index = 0;
                tmpI.pixel_format = tmpF.pixelformat;
                tmpI.width = tmpS.discrete.width;
                tmpI.height = tmpS.discrete.height;

                while( -1 != ioctl( m_fid, VIDIOC_ENUM_FRAMEINTERVALS, &tmpI ) )
                {
                    // make sure the discrete type is set
                    if((0 == tmpI.index) && (tmpI.type != V4L2_FRMIVAL_TYPE_DISCRETE) ) break;
                    if( tmpI.type != V4L2_FRMIVAL_TYPE_DISCRETE ) continue;

                    // save the frame rate
                    int fps = tmpI.discrete.denominator / tmpI.discrete.numerator;
                    tmpVM.fps.insert(fps);
                    tmpI.index++;
                }

                // only save if we found frame rates
                if( tmpVM.fps.size() > 0 ) m_modes.push_back(tmpVM);

                tmpS.index++;
            }
            tmpF.index++;
        }
        ret = true;
    }

    return ret;
}


bool LinuxCamera::enumMetadataModes()
{
    int offset = 0;
    bool ret = false;

    // clear the data structure
    m_metamode = 0;
    m_metasize = 0;

    // make sure fid is valid
    if( !isOpen() ) log( "Unable to call enumMetadataModes() as device is NOT open", warning );
    else
    {
        struct v4l2_format tmpF;

        memset( &tmpF, 0, sizeof(tmpF) );

        log( "ioctl(VIDIOC_G_FMT) metadata) for : " + m_userName, info );

        // walk the list of pixel formats, for each format, walk the list of video modes (sizes)
        tmpF.type = V4L2_BUF_TYPE_META_CAPTURE;

        if( -1 != ioctl(m_fid, VIDIOC_G_FMT, &tmpF ) )
        {            
            // log the return values
            m_metamode = tmpF.fmt.meta.dataformat;
            m_metasize = tmpF.fmt.meta.buffersize;

            char tmp[5];
            fourcc_int_to_charArray( m_metamode, tmp );
            tmp[4] = 0;
            ret = true;

        } else log( "ioctl(VIDIOC_G_FMT metadata) failed", warning );
    }

    return ret;
}

bool LinuxCamera::enumControls()
{
    bool ret = false;

    // clear the existing control structure
    m_controls.clear();

    // make sure fid is valid
    if( !isOpen() ) log( "Unable to call enumControls() as device is NOT open", warning );
    else
    {
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
                    m_controls[query_ext_ctrl.id].name = (char*)(query_ext_ctrl.name);
                    m_controls[query_ext_ctrl.id].id = query_ext_ctrl.id;
                    m_controls[query_ext_ctrl.id].value = query_ext_ctrl.default_value;
                    m_controls[query_ext_ctrl.id].type = query_ext_ctrl.type;
                    m_controls[query_ext_ctrl.id].typeStr = cntrlTypeToString(query_ext_ctrl.type);
                    m_controls[query_ext_ctrl.id].min = query_ext_ctrl.minimum;
                    m_controls[query_ext_ctrl.id].max = query_ext_ctrl.maximum;
                    m_controls[query_ext_ctrl.id].step = query_ext_ctrl.step;

                    // if this is a menu then grab the menu items
                    struct v4l2_querymenu querymenu;
                    memset (&querymenu, 0, sizeof (querymenu));
                    querymenu.id = query_ext_ctrl.id;

                    for (querymenu.index = query_ext_ctrl.minimum; querymenu.index <= query_ext_ctrl.maximum; querymenu.index++)
                    {
                        if (-1 != ioctl (m_fid, VIDIOC_QUERYMENU, &querymenu))
                        {
                            m_controls[query_ext_ctrl.id].menuItems[querymenu.index] = std::string((char*)(querymenu.name));
                        }
                    }

                }
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

        default:
            break;
    }

    return ret;
}
