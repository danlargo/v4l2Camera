#ifndef V4L2CAMERA_H
#define V4L2CAMERA_H

// Version History
//
// v0.2.101 : first recorded version, first commit to GITHUB
// v0.2.102 : creation of separate git project
// v0.2.103 : adding internal, tagged logging and serious refactoring
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
    static const int s_revision = 103;
    inline static const std::string s_codeName = "Karen";
    inline static const std::string s_lastCommitMsg = "[sjd] added logging, removed default dump to stdout, major refactor";

    static const int s_logDepth = 500;

    std::string m_fidName;
    std::string m_userName;

    int m_fid;

    unsigned int m_capabilities;
    struct video_mode m_currentMode;
    enum fetch_mode m_bufferMode;
    struct image_buffer * m_frameBuffer;

    std::string setTypeStr(int type);

    enum logging_mode m_logMode;
    std::vector<std::string> m_debugLog;

    std::map<int, user_control> m_controls;
    std::map<int, video_mode> m_modes;

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
    std::map<int, user_control> getControls() { return this->m_controls; };
    std::map<int, video_mode> getVideoModes() { return this->m_modes; };

    std::string getFidName();
    std::string getUserName();

    bool enumControls();
    bool enumVideoModes();
    bool enumCapabilities();

    int setValue( int id, int val );
    int getValue( int id );

    bool checkCapabilities( unsigned int val );

    void setLogMode( enum logging_mode );
    void clearLog();
    std::vector<std::string>getLogMsgs();

    bool isOpen();
    bool canOpen();

    bool canFetch();
    bool canRead();

    bool open();
    bool setFrameFormat( struct video_mode );
    bool init( enum fetch_mode );
    struct image_buffer * fetch( bool lastOne );
    void close();

};

#endif // V4L2CAMERA_H
