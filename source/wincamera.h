#ifndef WINCAMERA_H
#define WINCAMERA_H

#include "v4l2camera.h"

#include <map>
#include <vector>
#include <string>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfobjects.h>
#include <mftransform.h>
#include <mferror.h>
#include <mfmediaengine.h>

#define V4L2_CTRL_TYPE_INTEGER 1
#define V4L2_CTRL_TYPE_BOOLEAN 2
#define V4L2_CTRL_TYPE_MENU 3
#define V4L2_CTRL_TYPE_BUTTON 4
#define V4L2_CTRL_TYPE_INTEGER64 5
#define V4L2_CTRL_TYPE_CTRL_CLASS 6
#define V4L2_CTRL_TYPE_STRING 7
#define V4L2_CTRL_TYPE_BITMASK 8
#define V4L2_CTRL_TYPE_INTEGER_MENU 9
#define V4L2_CTRL_TYPE_U8 10
#define V4L2_CTRL_TYPE_U16 11
#define V4L2_CTRL_TYPE_U32 12

class WinCamera : public V4l2Camera
{
private:
    // device identifiers
    std::wstring m_devName;
    IMFSourceReader * m_pReader;
	IMFMediaSource* m_pSource;
    bool m_isOpen;

    int GetFourCCFromGUID(const GUID& guid);
    std::string GetLongNameFromGUID(const GUID& guid);
    const GUID GetGuidFromFourCC(int fourCC);
    std::string HResultToString(HRESULT hr);
    std::string cntrlIDToString(int id);

public:
    WinCamera( std::wstring devname );
    virtual ~WinCamera();

    static void initMF();
	static void shutdownMF();

    // Camera discovery methods
    //
    static std::vector<WinCamera *>  discoverCameras( v4l2cam_logging_mode logMode );

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

    virtual std::vector<std::string> capabilitiesToStr();


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

#endif // WINCAMERA_H
