#ifndef LINUXCAMERA_H
#define LINUXCAMERA_H

#include "v4l2camera.h"

#include <linux/videodev2.h>
    
#include <map>
#include <vector>
#include <string>

class LinuxCamera: public V4l2Camera
{
private:
    // device identifiers
    int m_fid;
    std::string m_devName;

public:
    LinuxCamera( std::string );
    virtual ~LinuxCamera();

    static std::vector<LinuxCamera *>  discoverCameras();
    static std::vector<std::string> buildCamList();

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

#endif // LINUXCAMERA_H
