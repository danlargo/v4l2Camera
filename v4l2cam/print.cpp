#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <chrono>

#include "defines.h"

#ifdef __linux__
    #include <unistd.h>
    #include "linuxcamera.h"
#elif __APPLE__
    #include <unistd.h>
    #include "macos/maccamera.h"
    #include "macos/v4l2cam_defs.h"
#elif _WIN32
    #include "wincamera.h"
    #include "v4l2cam_defs.h"
#endif

void printSudoHint( int numCameras )
{
    #ifdef __APPLE__
    if(  numCameras == 0 )
    {
        outwarn( "Hint : v4l2cam, may have to be run as root on MACOS to access USB cameras" );
        outwarn( "...it is not possible to ask for Camera permissions from a command line app on MACOS");
        outwarn( "...see Readme file for instructions on adding V4l2Camera to GUI app on MACOS" );
    }
    #endif

}
// Basic Helper Functions
void printVersionInfo()
{
    // ignore silentMode and output help message
    //
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
    // ignore silentMode and output help message
    //
    outln( "" );
    outln( "v4l2cam Usage" );
    outln( "-------------" );

    outln( "-h          :   this message" );
    outln( "-v          :   show version of the v4l2camera sub system");
    outln( "-V          :   operate in verbose mode (lots of information messages from low level functions)");
    outln( "-S          :   run totally silent so that this app will not be chatty when used in scripts");
    outln( "-x          :   display some sample commands");
    outln( "-!          :   display information about h264 encapulation mode" );
    outln( "---" );
    #ifdef __linux__
    outln( "-i          :   identify all openable devices in /dev/videoX driver space" );
    #endif
    outln( "-l          :   list all UVC compatible cameras in the USB device space" );
    outln( "-L          :   list only UVC compatible cameras that have streaming capability" );
    outln( "---" );
    outln( "-d [0..63]  :   select camera ## for operation" );
    outln( "-? [0..63]  :   display current video format and frame rate for camera ##" );
    outln( "---" );
    outln( "-m          :   list all the video modes supported by camera -d ##" );
    outln( "-u          :   list all the user controls supported by camera -d ##" );
    outln( "-M          :   grab the metadata associated with camera -d ##" );
    outln( "---" );
    outln( "-s [##] [val]   :   set user control [##] to value specified by [val] (do not include braces)");
    outln( "-r [##]     :   retrieve value from user control [##]");
    outln( "---" );
    outln( "-o file     :   specify filename for output, will send to stdout if not set" );
    outln( "-f fmt      :   specify output format for image, no attempt will be made to ensure the format matches the requested file extension");
    outln( "            :   ...   jpg - only supported if video mode is MJPEG");
    outln( "            :   ...   bmp - supported from all video modes except MJPEG");
    outln( "            :   ...   h264 - special encapulation for H264 video data, only supported in video capture mode");
    outln( "            :   ...   raw - output raw image data captured from camera, including MJPEG");
    outln( "            :   ...   any other fmt, image will be output as raw image data");
    outln( "            :   ...   if no fmt specified or not flag, image will be output as raw image data");
    outln( "-H          :   add header to H264 frames, only supported in video capture mode");
    outln( "---" );
    outln( "-F [val]    :   set the frame format for camera -d ##, to [val]" );
    outln( "-R [val]    :   set the frame rate for camera -d ##, to [val]" );
    outln( "-g          :   grab an image from camera -d ##" );
    outln( "-c          :   capture video from camera -d ##, for time -t [val] seconds, default is 10 seconds" );
    outln( "-t [val]    :   specify a time duration [val] for video capture, default is 10 seconds" );
    outln( "-T          :   run timing tests on current camera (-d #), with current video mode" );
    outln( "" );

    return 1;
}

void printExamples()
{
    // ignore silentMode and output help message
    //
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
