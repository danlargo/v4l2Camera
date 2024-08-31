#ifndef V4L2CAMERA_H
#define V4L2CAMERA_H

// Version History
//
// v0.2.101 : first recorded version, first commit to GITHUB
// v0.2.102 : creation of separate git project
// v0.2.103 : adding internal, tagged logging and serious refactoring
// v0.2.104 : refarctor, reverse if statements to meet driver specs ( if( -1 == function_call(...) ) instead of if( function_call == -1) ), ensures compiles will pick up missing double equal sign
//
//

#include <linux/videodev2.h>

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
    static const int s_minorVersion = 2;
    static const int s_revision = 104;
    inline static const std::string s_codeName = "Karen";
    inline static const std::string s_lastCommitMsg = "[sjd] refactor, change logic on ioctl calls to check for == -1 instead of > -1, check for failure, not for success";

    static const int s_logDepth = 500;

    std::string m_fidName;
    std::string m_userName;

    int m_fid;

    unsigned int m_capabilities;
    struct video_mode m_currentMode;
    enum fetch_mode m_bufferMode;
    struct image_buffer * m_frameBuffer;

    enum logging_mode m_logMode;
    std::vector<std::string> m_debugLog;

    std::map<int, struct user_control> m_controls;
    std::map<int, struct video_mode> m_modes;

    void log( std::string msg, enum msg_type tag = info );
    std::string getTagStr( enum msg_type );

public:
    V4l2Camera( std::string );
    ~V4l2Camera();

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

    bool enumControls();
    bool enumVideoModes();
    bool enumCapabilities();

    std::string cntrlTypeToString(int type);
    int setValue( int id, int val );
    int getValue( int id );

    bool checkCapabilities( unsigned int val );
    struct video_mode getOneVM( int index );
    struct user_control getOneCntrl( int index );

    void setLogMode( enum logging_mode );
    void clearLog();
    std::vector<std::string>getLogMsgs();

    bool isOpen();
    bool canOpen();

    bool canFetch();
    bool canRead();

    bool open();
    bool setFrameFormat( struct video_mode );
    bool setFrameFormat( std::string mode, int width, int height );
    bool init( enum fetch_mode );
    struct image_buffer * fetch( bool lastOne );
    void close();

};

#endif // V4L2CAMERA_H
