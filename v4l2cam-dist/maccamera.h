#ifndef MACCAMERA_H
#define MACCAMERA_H

#include "v4l2camera.h"
<<<<<<<< HEAD:v4l2cam-dist/maccamera.h
#include "v4l2cam_defs.h"
========
>>>>>>>> 442fbcc732c7adce19aec95983f6a0a1e20242b0:v4l2cam-dist/macos/maccamera.h

#include <map>
#include <vector>
#include <string>

class MACCamera : public V4l2Camera
{
public:
    MACCamera();
    virtual ~MACCamera();

<<<<<<<< HEAD:v4l2cam-dist/maccamera.h
========
    // libuvc support methods
    //
    static void initAPI();
    static void closeAPI();

>>>>>>>> 442fbcc732c7adce19aec95983f6a0a1e20242b0:v4l2cam-dist/macos/maccamera.h
    // Camera discovery methods
    //
    static std::vector<MACCamera *>  discoverCameras();

    // Methods that should be overridden in sublcass
    //
    virtual std::string getDevName();

    // Device access methods
    //
    virtual bool open();
    virtual bool init( enum v4l2cam_fetch_mode );
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
    virtual bool setFrameFormat( struct v4l2cam_video_mode );
    virtual struct v4l2cam_image_buffer * fetch( bool lastOne );

};

#endif // MACAMERA_H
