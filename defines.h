#include <string>

const std::string appName = "v4l2test";

bool verbose = false;

// command processors
void listUSBCameras();
void listAllDevices();
void listVideoModes( std::string );

// helper functions
void printBasicHelp();
void printVersionInfo();
void outln( std::string );
void outerr( std::string );

bool is_number(const std::string& s);