#ifndef MACCAMERA_H
#define MACCAMERA_H

#include "v4l2camera.h"

#include "i_objccamera.h"

#include <map>
#include <vector>
#include <string>

class MACCamera : public V4l2Camera
{
public:
    MACCamera( std::string device_name );
    virtual ~MACCamera();

    // device identifiers
    int m_fid;
    std::string m_devName;
    std::string m_manufacturer;
    std::string m_modelID;
    std::string m_deviceType;
    bool m_rearFacing;

    // Interface identifiers
    i_ObjCCamera * m_objcCamera;
    
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
