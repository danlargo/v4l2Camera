#ifndef LINUXCAMERA_H
#define LINUXCAMERA_H

#include "v4l2camera.h"

#ifdef __linux__
    #include <linux/videodev2.h>
#endif
    
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

    static std::map<int, LinuxCamera *>  discoverCameras();
    static std::vector<std::string> buildCamList();

    // Methods that should be overridden in sublcass
    virtual std::string cntrlTypeToString(int type) override;
    virtual std::string getDevName() override;
    virtual bool canFetch() override;
    virtual bool canRead() override;
    virtual bool enumControls() override;
    virtual bool enumVideoModes() override;
    virtual bool enumCapabilities() override;
    virtual int setValue( int id, int val, bool openOnDemand = false ) override;
    virtual int getValue( int id, bool openOnDemand = false ) override;
    virtual bool isOpen() override;
    virtual bool canOpen() override;
    virtual std::string getCameraType() override;
    virtual bool open() override;
    virtual bool setFrameFormat( struct video_mode ) override;
    virtual bool init( enum fetch_mode ) override;
    virtual void close() override;
    virtual struct image_buffer * fetch( bool lastOne ) override;

};

#endif // LINUXCAMERA_H
