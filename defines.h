#include <string>

const std::string appName = "v4l2test";

bool verbose = false;

// command processors
void listUSBCameras();
void listAllDevices();
void listVideoModes( std::string );
void snapImage( std::string, std::string, std::string );
void captureVideo( std::string, std::string, std::string, std::string );

// helper functions
int printBasicHelp();
void printVersionInfo();

std::map<std::string, std::string> parseCmdLine( int argc, char** argv );

void outln( std::string );
void outerr( std::string );
void outinfo( std::string );

bool is_number(const std::string& s);