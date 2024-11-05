#ifndef MACCAMERA_H
#define MACCAMERA_H

#include "uvccamera.h"
#include <libusb-1.0/libusb.h>

    struct usb_device
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

class MACCamera : public UVCCamera
{

protected:
    struct usb_device * m_dev;
    libusb_device_handle * m_Handle;

public:
    MACCamera( struct usb_device * dev );
    virtual ~MACCamera();

    static void initAPI();
    static void closeAPI();

    static std::map<int, MACCamera *>  discoverCameras();
    static std::vector<struct usb_device *> buildCamList();

    // Methods that should be overridden in sublcass
    virtual std::string cntrlTypeToString(int type);
    virtual std::string getDevName();
    virtual bool canFetch();
    virtual bool canRead();
    virtual bool enumControls();
    virtual bool enumVideoModes();
    virtual bool enumCapabilities();
    virtual int setValue( int id, int val, bool openOnDemand = false );
    virtual int getValue( int id, bool openOnDemand = false );
    virtual bool isOpen();
    virtual bool canOpen();
    virtual std::string getCameraType();
    virtual bool open();
    virtual bool setFrameFormat( struct video_mode );
    virtual bool init( enum fetch_mode );
    virtual void close();
    virtual struct image_buffer * fetch( bool lastOne );

};

#endif // MACAMERA_H
