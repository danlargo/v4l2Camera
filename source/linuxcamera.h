#ifndef LINUXCAMERA_H
#define LINUXCAMERA_H

#include "v4l2camera.h"

#include <linux/videodev2.h>
    
#include <vector>
#include <string>

# define NUM_QBUF 5

class LinuxCamera: public V4l2Camera
{
private:
    // device identifiers
    int m_fid;
    std::string m_devName;
    struct v4l2_buffer buf[NUM_QBUF];

public:
    LinuxCamera( std::string );
    virtual ~LinuxCamera();

    static std::vector<LinuxCamera *>  discoverCameras(v4l2cam_logging_mode logMode, bool streamingOnly = false);
    static std::vector<std::string> buildCamList();

    // Methods that should be overridden in sublcass
    virtual std::string getDevName() override;
    virtual bool enumCapabilities() override;
    virtual bool canFetch() override;
    virtual bool canRead() override;
    virtual bool hasMetaData() override;

    virtual std::vector<std::string> capabilitiesToStr() override;

    virtual bool enumControls() override;
    virtual std::string cntrlTypeToString(int type) override;
    virtual int setValue( int id, int val, bool openOnDemand = false ) override;
    virtual int getValue( int id, bool openOnDemand = false ) override;

    virtual bool enumVideoModes() override;
    virtual bool setFrameFormat( std::string mode, int width, int height, int fps = 30 ) override;
    virtual bool setFrameFormat( struct v4l2cam_video_mode, int fps = 30 ) override;
    virtual struct v4l2cam_video_mode * getFrameFormat() override;
    virtual int getFrameRate() override;
    virtual bool setFrameRate( int fps ) override;

    virtual bool enumMetadataModes() override;

    virtual bool isOpen() override;
    virtual bool open() override;
    virtual bool init( enum v4l2cam_fetch_mode ) override;
    virtual void close() override;

    virtual struct v4l2cam_image_buffer * fetch( bool lastOne ) override;
    virtual struct v4l2cam_metadata_buffer * fetchMetaData() override;

};

#endif // LINUXCAMERA_H
