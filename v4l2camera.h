#ifndef V4L2CAMERA_H
#define V4L2CAMERA_H

// Version History
//
// v0.2.101 : first recorded version, first commit to GITHUB
// v0.2.102 : creation of separate git project
//
//

#include <linux/videodev2.h>

#include <map>
#include <string>

struct devControl
{
    std::string name;
    int type;
    std::string typeStr;
    int min;
    int max;
    int step;
    int value;
};

struct videoMode
{
    int index;
    unsigned int pixelFormat;
    std::string pixelStr;
    int width;
    int height;
    int imgSize;
};

struct imageBuffer
{
    int length;
    unsigned char * buffer;
};

enum fetchMode
{
    notset, readMode, userPtrMode, mMapMode
};

class v4l2Camera
{
private:
    static const int majorVersion = 0;
    static const int minorVersion = 2;
    static const int revision = 102;
    inline static const std::string codeName = "Karen";
    inline static const std::string lastCommitMsg = "[sjd] Moved v4l2Camera files standalone github project";

    std::string fidName, devName;
    unsigned int myCaps;
    struct videoMode curMode;
    int fid;
    enum fetchMode myFetchMode;
    struct imageBuffer * frameBuffer;

    std::string setTypeStr(int type);

public:
    static std::string getVersionString() { 
        std::string ver = "v"; 
        ver += std::to_string(majorVersion); ver += "."; 
        ver += std::to_string(minorVersion); ver += "."; 
        ver += std::to_string(revision);  
        return ver;
        }
    static std::string getCodeName() { return "(" + codeName +")"; }
    static std::string getLastMsg() { return lastCommitMsg; }

    std::map<int, devControl> myControls;
    std::map<int, videoMode> myModes;

    v4l2Camera( std::string devName );

    std::string getFidName();
    std::string getDevName();

    bool enumControls();
    bool enumVideoModes();

    int setValue( int id, int val );
    int getValue( int id );

    bool getCaps();

    bool isOpen();
    bool canOpen();

    bool canFetch();
    bool canFetch_Read();

    bool open();
    bool setFrameFormat( struct videoMode );
    bool init( enum fetchMode );
    struct imageBuffer * fetch( bool lastOne );
    void close();

};

#endif // V4L2CAMERA_H
