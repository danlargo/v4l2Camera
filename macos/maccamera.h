#ifndef MACCAMERA_H
#define MACCAMERA_H

#include "../base/v4l2camera.h"
#include "libuvc/libuvc.h"

struct uvc_device
{
    int bus;
    int address;
    int vid;
    int pid;
    std::string productName;
    std::string manufacturerName;
    std::string serialNumber;
};

#include <map>
#include <vector>
#include <string>

static uvc_context_t * UVC_ctx;

static std::map<int, struct v4l2_control_defs> controlDefs;


class MACCamera : public V4l2Camera
{
public:
    MACCamera( struct uvc_device * dev );
    virtual ~MACCamera();

    // Device access vaariables
    //
    struct uvc_device * m_dev;
    struct uvc_device_handle * m_Handle;

    // libuvc support methods
    //
    static void initAPI();
    static void closeAPI();
    static void buildControlDefList();

    // Camera discovery methods
    //
    static std::vector<MACCamera *>  discoverCameras();
    static std::vector<struct uvc_device *> buildCamList();

    // Methods that should be overridden in sublcass
    //
    virtual std::string getDevName();

    // Device access methods
    //
    virtual bool open();
    virtual bool init( enum v4l2_fetch_mode );
    virtual void close();


    // Collect Camera information
    //
    virtual bool enumCapabilities();
    virtual bool enumControls();
    virtual bool enumVideoModes();
    virtual bool isOpen();

    // Check and change camera settings
    //
    virtual bool canFetch();
    virtual bool canRead();
    virtual std::string cntrlTypeToString(int type);
    virtual int setValue( int id, int val, bool openOnDemand = false );
    virtual int getValue( int id, bool openOnDemand = false );

    // Image fetch methods
    //
    virtual bool setFrameFormat( struct v4l2_video_mode );
    virtual struct v4l2_image_buffer * fetch( bool lastOne );

};

#endif // MACAMERA_H
