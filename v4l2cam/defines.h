#include <string>
#include <map>

// v4l2cam - demo app for V4l2Camera class
//
const std::string appName = "v4l2cam";

enum h264FrameType_t
{
    H264_FRAME_I = 0x05,
    H264_FRAME_P = 0x01,
    H264_FRAME_B = 0x02
};

#pragma pack(push, 1)
struct h264FrameHeader_t
{
    unsigned char delimiter[4];
    unsigned int frame_type;
    unsigned int sps_offset;
    unsigned int pps_offset;
    unsigned int frame_offset;
    unsigned int frame_size;
};
#pragma pack(pop)

// OS specific includes
//
void printSudoHint( int numCameras );

// Logging control
//
// encourage the low level objects to spew out information
extern bool verbose;
// run totally silent so that v4l2cam can be used in scripts
extern bool silentMode;

// Control how message is displayed
//
void outln( std::string );
void outerr( std::string );
void outinfo( std::string );

// Command processors
//
void listUSBCameras();
void listAllDevices();
void listVideoModes( std::string deviceID );
void listUserControls( std::string deviceID );
void fetchMetaData( std::string deviceID );
void getVideoFormat( std::string deviceID );

void runTimingTest( std::string deviceID );

void captureImage(std::string deviceID, std::string fileName = "", std::string format = "", std::string addHeader = "" );
void captureVideo( std::string deviceID, std::string timeInSeconds = "10", std::string fileName = "", std::string addHeader = "" );   

char * addH264Header( unsigned char * buffer, int length );

void setVideoFormat( std::string deviceID, std::string videoMode, std::string fps );
void setFrameRate( std::string deviceID, std::string fps );

int getControlValue( std::string deviceID, std::string cntrlID );
bool setControlValue( std::string deviceID, std::string cntrlID, std::string newVal );

// Helper functions
//
int printBasicHelp();
void printVersionInfo();
void printExamples();

std::map<std::string, std::string> parseCmdLine( int argc, char** argv );
bool is_number(const std::string& s);
std::string makeHexString( unsigned char * buf, int len, bool makeCaps );

// Endian Swapping
//
unsigned int swapEndian( unsigned int in );
int swapEndian( int in );
unsigned short swapEndian( unsigned short in );
unsigned long swapEndian( unsigned long in );