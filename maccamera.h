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
    };
#endif

#include <map>
#include <vector>
#include <string>

struct user_control
{
    std::string name;
    int type;
    std::string typeStr;
    int min;
    int max;
    std::map<int, std::string> menuItems;
    int step;
    int value;
};

struct video_mode
{
    int index;
    unsigned int fourcc;
    std::string format_str;
    int width;
    int height;
    int size;
};

struct image_buffer
{
    int length;
    int width;
    int height;
    unsigned char * buffer;
};

enum fetch_mode
{
    notset, readMode, userPtrMode, mMapMode
};

enum logging_mode
{
    logOff, logInternal, logToStdErr, logToStdOut
};

enum msg_type
{
    info, warning, error, critical
};

class V4l2Camera
{
private:
    static const int s_majorVersion = 0;
    static const int s_minorVersion = 3;
    static const int s_revision = 101;
    inline static const std::string s_codeName = "Andrea";
    inline static const std::string s_lastCommitMsg = "[sjd] adding support for MacOS using libusb";

    static const int s_logDepth = 500;

    // device identifiers
    #ifdef __linux__
        int m_fid;
        std::string m_fidName;
    #elif __APPLE__
        struct usb_device * m_dev;
        libusb_device_handle * m_Handle;
    #endif

    unsigned int m_capabilities;
    struct video_mode m_currentMode;
    struct image_buffer * m_frameBuffer;

    enum logging_mode m_logMode;
    std::vector<std::string> m_debugLog;

    std::map<int, struct user_control> m_controls;
    std::map<int, struct video_mode> m_modes;

    void log( std::string msg, enum msg_type tag = info );
    std::string getTagStr( enum msg_type );

protected:
    std::string m_userName;
    enum fetch_mode m_bufferMode;

public:
    #ifdef __linux__
        V4l2Camera( std::string );
    #elif __APPLE__
        V4l2Camera( struct usb_device * dev );
    #endif

    virtual ~V4l2Camera();

    static std::string getVersionString() 
    { 
        std::string ver = "v"; 
        ver += std::to_string(s_majorVersion); ver += "."; 
        ver += std::to_string(s_minorVersion); ver += "."; 
        ver += std::to_string(s_revision);  
        return ver;
    }

    static std::string getCodeName() { return "(" + s_codeName +")"; }
    static std::string getLastMsg() { return s_lastCommitMsg; }

    std::map<int, struct user_control> getControls() { return this->m_controls; };
    std::map<int, struct video_mode> getVideoModes() { return this->m_modes; };

    std::string getFidName();
    std::string getUserName();

    static std::map<int, V4l2Camera *>  discoverCameras();
    static void initAPI();
    static void closeAPI();
    
    #ifdef __linux__
        static std::vector<std::string> buildCamList_dev();
    #elif __APPLE__
        static std::vector<struct usb_device *> buildCamList_usb();
        libusb_device_handle * openCamera_usb( int bus, int address, int vid, int pid );
    #endif

    bool enumControls();
    bool enumVideoModes();
    bool enumCapabilities();

    std::string cntrlTypeToString(int type);
    int setValue( int id, int val, bool openOnDemand = false );
    int getValue( int id, bool openOnDemand = false );

    bool checkCapabilities( unsigned int val );
    struct video_mode getOneVM( int index );
    struct user_control getOneCntrl( int index );

    void setLogMode( enum logging_mode );
    void clearLog();
    std::vector<std::string>getLogMsgs();

    bool canFetch();
    bool canRead();

    // Methods that should be overridden in sublcass
    virtual bool isOpen();
    virtual bool canOpen();
    virtual std::string getCameraType();
    virtual bool open();
    virtual bool setFrameFormat( struct video_mode );
    virtual bool init( enum fetch_mode );
    virtual void close();
    virtual bool setFrameFormat( std::string mode, int width, int height );
    virtual struct image_buffer * fetch( bool lastOne );

};

#endif // V4L2CAMERA_H
