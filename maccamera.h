#ifndef MACCAMERA_H
#define MACCAMERA_H

#include "v4l2camera.h"
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

class MACCamera : public V4l2Camera
{

protected:
    struct uvc_device * m_dev;
    struct uvc_device_handle * m_Handle;

public:
    MACCamera( struct uvc_device * dev );
    virtual ~MACCamera();

    static std::vector<MACCamera *>  discoverCameras();
    static std::vector<struct uvc_device *> buildCamList();

    static void initAPI();
    static void closeAPI();

    // Methods that should be overridden in sublcass
    virtual std::string getDevName();
    virtual bool enumCapabilities();
    virtual bool canFetch();
    virtual bool canRead();

    virtual bool enumControls();
    virtual std::string cntrlTypeToString(int type);
    virtual int setValue( int id, int val, bool openOnDemand = false );
    virtual int getValue( int id, bool openOnDemand = false );

    virtual bool enumVideoModes();
    virtual bool setFrameFormat( struct video_mode );

    virtual bool isOpen();
    virtual std::string getCameraType();

    virtual bool open();
    virtual bool init( enum fetch_mode );
    virtual void close();

    virtual struct image_buffer * fetch( bool lastOne );

};

#endif // MACAMERA_H
