#include <string>

const std::string appName = "v4l2test";

// encourage the low level objects to spew out information
bool verbose = false;

// run totally silent so that v4l2Test can be used in scripts
bool silentMode = false;

// command processors
void listUSBCameras();
void listAllDevices();
void listVideoModes( std::string deviceID );
void listUserControls( std::string deviceID );
void grabImage( std::string deviceID, std::string videoMode, std::string fileName = "" );
void captureVideo( std::string deviceID, std::string videoMode, std::string timeInSeconds = "10", std::string fileName = "");

// helper functions
int printBasicHelp();
void printVersionInfo();
void printExamples();

std::map<std::string, std::string> parseCmdLine( int argc, char** argv );

void outln( std::string );
void outerr( std::string );
void outinfo( std::string );

bool is_number(const std::string& s);
std::string makeHexString( unsigned char * buf, int len, bool makeCaps );