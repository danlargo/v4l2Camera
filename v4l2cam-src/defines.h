#include <string>
#include <map>

// v4l2cam - demo app for V4l2Camera class
//
const std::string appName = "v4l2cam";

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

void captureImage(std::string deviceID, std::string videoMode, std::string fileName = "", std::string format = "" );
void captureVideo( std::string deviceID, std::string videoMode, std::string timeInSeconds = "10", std::string fileName = "");

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