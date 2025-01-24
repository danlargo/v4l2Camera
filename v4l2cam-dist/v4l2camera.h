#ifndef V4L2CAMERA_H
#define V4L2CAMERA_H

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
// v0.3.101 : creating super-class V4L2Camera and sub-classes LinuxCamera and MACCamera
// v0.3.102 : enumerate UVC cameras, via libusb on MACOS
// v0.3.103 : added getVideoMode support on MACOS, and fixed up some refactoring on Linux side
// v0.3.104 : restructured folder hierarchy, started adding control support on MACOS
// v0.3.105 : restructured build folders, create library for export and linking to other apps, added isHealthy() to measure I/F health
// v0.3.106 : added support for user controls on MACOS
// v0.3.107 : backed out all MacOS changes, added folder structure for MacOS and Windows
// v0.3.108 : updated documentation, converted repo to Public
// v1.0.100 : First release build
// v1.1.101 : starting to add MacOS support via AVFoundation framework
// v1.1.110 : enhancing capability ennumeration
// v1.3.010 : started adding Windows support

#include <map>
#include <vector>
#include <string>

// Control structures
//

// v4l2_control - structure to hold a single camera control
//
struct v4l2cam_control
{
    std::string name;
    int type;
    int id;
    std::string typeStr;
    int min;
    int max;
    std::map<int, std::string> menuItems;
    int step;
    int value;
};

// v4l2_video_mode - structure to hold a single video mode
//
struct v4l2cam_video_mode
{
    unsigned int fourcc;
    std::string format_str;
    int width;
    int height;
    int size;
};

// v4l2_image_buffer - structure to hold a single image buffer
//
struct v4l2cam_image_buffer
{
    int length;
    int width;
    int height;
    unsigned char * buffer;
};

// v4l2_metadata_buffer - structure to hold meta data buffer
//
struct v4l2cam_metadata_buffer
{
    int length;
    int errcode;
    unsigned char * buffer;
};

// Image Fetch Mode, only userPtrMode is supported
//
enum v4l2cam_fetch_mode
{
    notset, readMode, userPtrMode, mMapMode
};

// Logging control - indicates where information messages are displayed
//
enum v4l2cam_logging_mode
{
    logOff, logInternal, logToStdErr, logToStdOut
};

// Logging message type - indicates the severity of the message
//
enum v4l2cam_msg_type
{
    info, warning, error, critical
};

// V4l2Camera - base class for all camera types
//
class V4l2Camera
{
private:
    // Version Info
    //
    static const int s_majorVersion = 1;
    static const int s_minorVersion = 3;
    static const int s_revision = 010;
    inline static const std::string s_codeName = "Tara";
    inline static const std::string s_lastCommitMsg = "[danlargo] started adding Windows support";

    static const int s_logDepth = 500;

public:

    // Super class contructor and destructor
    V4l2Camera();
    virtual ~V4l2Camera();

    // Version Information helpers
    //
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

    // health tracker info
    static const int s_healthCountLimit = 10;

    // Common class variables for all sub-classes
    //
    // defines camera capabilities
    //
    std::map<int, struct v4l2cam_control> m_controls;
    std::vector<struct v4l2cam_video_mode> m_modes;
    unsigned int m_metamode, m_metasize;

    unsigned int m_capabilities;
    std::string m_userName;
    std::string m_cameraType;

    // Operational variables
    //
    struct v4l2cam_image_buffer * m_frameBuffer;
    struct v4l2cam_video_mode m_currentMode;
    enum v4l2cam_fetch_mode m_bufferMode;
    int m_healthCounter;

    // Logging control
    //
    enum v4l2cam_logging_mode m_logMode;
    std::vector<std::string> m_debugLog;
    void log( std::string msg, enum v4l2cam_msg_type tag = v4l2cam_msg_type::info );
    void setLogMode( enum v4l2cam_logging_mode );
    void clearLog();
    std::vector<std::string>getLogMsgs( int num );
    std::string getTagStr( enum v4l2cam_msg_type );


    // Camera capabilities getters
    //
    std::string getUserName();
    std::string getCameraType();

    std::map<int, struct v4l2cam_control> getControls() { return this->m_controls; };
    struct v4l2cam_control getOneCntrl( int index );

    std::vector<struct v4l2cam_video_mode> getVideoModes() { return this->m_modes; };
    struct v4l2cam_video_mode getOneVM( int index );

    int getMetaMode() { return m_metamode; };
    int getMetaSize() { return m_metasize; };

    bool checkCapabilities( unsigned int val );

    bool setFrameFormat( std::string mode, int width, int height );


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
    virtual bool enumMetadataModes();
    virtual bool isOpen();
    virtual bool isHealthy();

    virtual std::vector<std::string> capabilitiesToStr();

    // Check and change camera settings
    //
    virtual bool canFetch();
    virtual bool canRead();
    virtual bool hasMetaData();

    virtual std::string cntrlTypeToString(int type);
    virtual int setValue( int id, int val, bool openOnDemand = false );
    virtual int getValue( int id, bool openOnDemand = false );

    // Image fetch methods
    //
    virtual bool setFrameFormat( struct v4l2cam_video_mode );
    virtual struct v4l2cam_image_buffer * fetch( bool lastOne );

    // Meta Data methods
    //
    virtual struct v4l2cam_metadata_buffer * fetchMetaData();

    // FourCC conversion methods
    //
    void fourcc_int_to_charArray( unsigned int fourcc, char * ret );
    unsigned int fourcc_charArray_to_int( unsigned char * fourcc );
    unsigned int fourcc_intArray_to_int( int * fourcc );
    std::string fourcc_int_to_descriptor( unsigned int fourcc );

};

#endif // V4L2CAMERA_H
