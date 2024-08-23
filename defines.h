#include <string>

const std::string appName = "v4l2test";

// command processors
void listUSBCameras();
void listAllDevices();

// helper functions
void invalidCommandMsg( std::string );
void printBasicHelp();
void printVersionInfo();
void outln(  std::string );
