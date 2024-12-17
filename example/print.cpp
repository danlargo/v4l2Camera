#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <chrono>

#include "defines.h"

#ifdef __linux__
    #include "linuxcamera.h"
#elif __APPLE__
    #include "maccamera.h"
    #include "v4l2cam_defs.h"
#endif

#include <unistd.h>

void printSudoHint( int numCameras )
{
    #ifdef __APPLE__
    if(  numCameras == 0 )
    {
        outln( "Hint : v4l2cam, may have to be run as root on MACOS to access USB cameras" );
        outln( "...it is not possible to ask for Camera permissions from a command line app on MACOS");
        outln( "...see Readme file for instructions on adding V4l2Camera to GUI app on MACOS" );
    }
    #endif

}
// Basic Helper Functions
void printVersionInfo()
{
    outln("");
    outln( "V4l2Camera - version info");
    outln( "-------------------------");
    outln( "Version     : " + V4l2Camera::getVersionString() );
    outln( "Code Name   : " + V4l2Camera::getCodeName() );
    outln( "Last Commit : " + V4l2Camera::getLastMsg() );
    outln( "" );
}

int printBasicHelp()
{
    outln( "" );
    outln( "v4l2cam Usage" );
    outln( "-------------" );

    outln( "-h :            this message" );
    outln( "-v :            show version of the UVCCamera sub system");
    outln( "-V :            operate in verbose mode (lots of information messages from low level functions)");
    outln( "-S :            run totally silent so that this app will not be chatty when used in scripts");
    outln( "-x :            display some sample commands");
    outln( "---" );
    #ifdef __linux__
        outln( "-i :            identify all openable devices in /dev/videoX driver space" );
    #endif
    outln( "-l :            list all UVC compatible cameras in the USB device space" );
    outln( "---" );
    outln( "-d [0..63] :    select camera # for operation" );
    outln( "-m :            list all the video modes supported by camera -d #" );
    outln( "-u :            list all the user controls supported by camera -d #" );
    outln( "---" );
    outln( "-t [0..##] :    set user control [0..??] to value specified with -k [##]");
    outln( "-k [0..##] :    used user control number [0..##] for set or retrieve commands");
    outln( "-r :            retrieve value from user control specified by -k [##]");
    outln( "-o file    :    specify filename for output, will send to stdout if not set" );
    outln( "---" );
    outln( "-g [0..##] :    grab an image from camera -d #, using video mode <number>" );
    outln( "-c [0..##] :    capture video from camera -d #, using video mode <number>, for time -t [0..##] seconds, default is 10 seconds" );
    outln( "-t [0..##] :    specify a time duration for video capture, default is 10 seconds" );
    outln( "" );

    return 1;
}

void printExamples()
{
    outln("");
    outln( "v4l2cam - example usage");
    outln( "-------------------------");
    outln( "...skipping al the obvious examples");
    outln( "" );
    outln( "...get video modes for a camera 0");
    outln( "$ ./v4l2cam -m -d 0");
    outln( "" );
    outln( "...list user controls for camera 2");
    outln( "$ ./v4l2cam -u -d 0");
    outln( "" );
    outln( "...grab an image from casmera 4, using image mode 2, save to <test.jpg>");
    outln( "$ ./v4l2cam -g 2 -d 4 -o test.jpg");
    outln( "" );
    outln( "...capture video from  camera 2, using video mode 1, stream to stdout, pipe to test.mp4");
    outln( "$ ./v4l2cam -c 1 -d 2 > ./test.mp4");
    outln( "" );
    outln( "...capture video from device and send directly to ffmpeg for processing");
    outln( "$ ./v4l2cam -c 7 -d 2 -t 5 | ffmpeg -r 30 -i pipe: ../test3.mp4");
    outln( "" );
    outln( "...get the value from /dev/video2, for user control 9963776 (brightness)");
    outln( "$ ./v4l2cam -r -k 9963776 -d 2");
    outln( "" );
    outln( "...set the value for /dev/video2, for user control 9963776 to 25");
    outln( "$ ./v4l2cam -t 25 -k 9963776 -d 2");
    outln( "" );
}
