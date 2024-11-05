#ifndef UVCCAMERA_H
#define UVCCAMERA_H

// Version History
//
// v0.2.101 : first recorded version, first commit to GITHUB
// v0.2.102 : creation of separate git project
// v0.2.103 : adding internal, tagged logging and serious refactoring
// v0.2.104 : refarctor, reverse if statements to meet driver specs ( if( -1 == function_call(...) ) instead of if( function_call == -1) ), ensures compiles will pick up missing double equal sign
//
// v0.2.105 : added support for sub classing, to allow handling of custom camera APIs
// v0.2.106 : minor enhancements, decided to remove MMAP data access support
// v0.2.107 : enhanced the v4l2cam app, added image and video capture
//
// v0.3.100 : adding support for MacOS, using libusb instead of video4linux2
// v0.3.101 : creating super-class UVCCamera and sub-classes v4l2Camera and MACCamera

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

class UVCCamera
{
private:
    static const int s_majorVersion = 0;
    static const int s_minorVersion = 3;
    static const int s_revision = 102;
    inline static const std::string s_codeName = "Andrea";
    inline static const std::string s_lastCommitMsg = "[sjd] creating super class to clean up OS support differences";

    static const int s_logDepth = 500;

protected:
    struct image_buffer * m_frameBuffer;
    std::map<int, struct user_control> m_controls;
    std::map<int, struct video_mode> m_modes;

    enum logging_mode m_logMode;
    std::vector<std::string> m_debugLog;
    void log( std::string msg, enum msg_type tag = msg_type::info );

    unsigned int m_capabilities;
    struct video_mode m_currentMode;

    std::string getTagStr( enum msg_type );

    std::string m_userName;
    enum fetch_mode m_bufferMode;

public:
    UVCCamera();
    virtual ~UVCCamera();

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

    std::string getUserName();


    bool checkCapabilities( unsigned int val );
    struct video_mode getOneVM( int index );
    struct user_control getOneCntrl( int index );

    bool setFrameFormat( std::string mode, int width, int height );

    void setLogMode( enum logging_mode );
    void clearLog();
    std::vector<std::string>getLogMsgs();

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

#endif // UVCCAMERA_H
