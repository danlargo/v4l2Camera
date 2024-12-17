#include <string>

struct avCamera
{
    std::string uniqueID;
    std::string name;
    std::string modelID;
    std::string manufacturer;
    std::string deviceType;
    bool rearFacing;
};

struct avFormat
{
    int fourCC;
    std::string mediaType;
    int width;
    int height;
};

class i_ObjCCamera
{
public:
    i_ObjCCamera();
    ~i_ObjCCamera();
    bool open( std::string device_name );
    bool close();
    std::vector<struct avFormat*> getVideoFormats();
    std::vector<struct avCamera*> discoverCameras();

    bool setCaptureFormat( int i );

    int reverseBytes( int in );

private:
    void * m_objcCamera;
};