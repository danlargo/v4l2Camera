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

// Functional demonstration functions
void listUSBCameras()
{
    outinfo( "" );
    outinfo( "USB camera discovery starting..." );

    #ifdef __linux__
        std::vector<LinuxCamera *> camList;
        camList = LinuxCamera::discoverCameras();
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
    #endif

    // output the collected information
    outln( "" );
    outln( "Detected " + std::to_string(camList.size()) + " USB camera(s)" );
    outln( "" );
    printSudoHint( camList.size() );

    int camIndex = 0;
    for( const auto &x: camList )
    {
        if( verbose ) x->setLogMode( v4l2cam_logging_mode::logToStdOut );
        else x->setLogMode( v4l2cam_logging_mode::logOff );

        if( x )
        {
            if( x->open() )
            {
                outln( "[" + std::to_string(camIndex++) + "] "
                        + x->getDevName() + " : " + x->getUserName() + ", " 
                        + std::to_string(x->getVideoModes().size()) + " video modes, "
                        + std::to_string(x->getControls().size()) + " user controls"
                    );
            }
            // close the camera now that we are done with it
            x->close();
        } else outerr( "Invalid V4l2Camera pointer in list" );
    }
    // delete all the cameras
    for( const auto &x : camList ) delete x;

    #ifdef __linux__
        outln( "" );
        outln( "---------------------------------------");
        outln( "Extra info on USB cameras and device IDs :");
        outln( "" );
        outln( " You will notice that the USB cameras tend to exist on even numbers,");
        outln( " when you look at the lower level details the cameras will actually exists on two devices (ex, video0 and video1)" );
        outln( " Only the even number device will actual produce any results from ioctl queries, no controls or video modes are");
        outln( " on the odd number devices.");
        outln( "" );
        outln( " ...according to Stack Overflow - This was added in kernel 4.16 - the second device provides metadata. ");
        outln( " ...see unix.stackexchange.com/a/539573/13613 for more details.");
        outln( " ...looks lke someting to add in future versions of v4l2Camera");
    #endif

    outln( "" );
    outln( "...discovery complete" );
}

void listAllDevices()
{
    #ifdef __linux__
        outln( "" );
        outln( "Devices in range /dev/video[0..63]" );
        outln( "----------------------------------" );

        int numFound = 0;

        for( int i=0; i<64; i++ )
        {

            // build the device name
            std::string nam = "/dev/video" + std::to_string(i);

            // create the camera object
            V4l2Camera * tmpC = new LinuxCamera(nam);
            if( verbose ) tmpC->setLogMode( v4l2cam_logging_mode::logToStdOut );
            else tmpC->setLogMode( v4l2cam_logging_mode::logOff );

            if( tmpC )
            {
                if( tmpC->open() ) 
                {
                    outln( nam + " : can be opened" );
                    numFound++;
                    tmpC->close();
                }
            } else outerr( nam + " : error !!! - failed to create V4l2Camera object for device" );

            delete tmpC;
        }

        outln( "" );
        outln( "...found : " + std::to_string(numFound) + " devices that can be opened by user" );
        outln( "" );
        outln( "...discovery complete" );

    #elif __APPLE__
        outln( "*** Not supported on MACOS" );
        outln( "" );
    #endif
}

void listVideoModes( std::string deviceID )
{
    // create the camera object
    #ifdef __linux__
        std::vector< LinuxCamera *> camList;
        camList = LinuxCamera::discoverCameras();
        LinuxCamera * tmp = nullptr;
        if( (camList.size() > 0) && (std::stoi(deviceID) < camList.size()) ) tmp = camList[std::stoi(deviceID)];
        else 
        {
            outerr( "Selected device outside range : numCameras = " + std::to_string(camList.size()) + " yourDeviceID = " + deviceID );
            return;
        }
    #elif __APPLE__
        std::vector< MACCamera *> camList;
        MACCamera * tmp = nullptr;
        camList = MACCamera::discoverCameras();
        if( (camList.size() > 0) && (std::stoi(deviceID) < camList.size()) ) tmp = camList[std::stoi(deviceID)];
        else 
        {
            outerr( "Selected device outside range : numCameras = " + std::to_string(camList.size()) + " yourDeviceID = " + deviceID );
            return;
        }
        printSudoHint( camList.size() );
    #endif

    outln( "" );
    outln( "------------------------------");
    outln( "Video Mode(s) for camera [" + deviceID + "] " + tmp->getDevName() + " : " + tmp->getUserName() );

    if( verbose ) tmp->setLogMode( v4l2cam_logging_mode::logToStdOut );
    else tmp->setLogMode( v4l2cam_logging_mode::logOff );

    if( tmp )
    {
        if( tmp->open() )
        {
            std::vector<v4l2cam_video_mode> modes = tmp->getVideoModes();
            outln( "" );
            outln( "...Found " + std::to_string(modes.size()) + " video modes, details following..." );
            outln( "" );

            int offset = 0;
            for( auto x : tmp->getVideoModes() )
            {
                struct v4l2cam_video_mode vm = x;
                outln( std::to_string(offset++) + " - " + vm.format_str + " : " + std::to_string(vm.width) + " x " + std::to_string(vm.height) );
            }

            tmp->close();
        }
    }
    // delete all the camera object now that we are done with them
    for( const auto &x : camList ) delete x;
    
    #ifdef __linux__
        outln( "");
        outln( "---------------------------------------");
        outln( "" );
        outln( "Extra info on saving video mode frames to a file :");
        outln(  "");
        outln( "Motion-JPEG" );
        outln( "    - single frame JPG images at specified resolution");
        outln( "    - can be saved as <.jpg> files with single image capture");
        outln( "    - can be saved to <.mjpg> files as multi-frame video, may not play properly until run thru <ffmpeg> to be cleaned up");
        outln( "    - this line will fix up the video (v4l2cam uses 30 fps as a default) : ffmpeg -r 30 -i ../test.mjpg ../test2.mp4");
        outln( "    - in fact this line will do it all in one step : ./v4l2cam -c 7 -d 2 -t 5 | ffmpeg -r 30 -i pipe: ../test3.mp4");
        outln( "" );
        outln( "YUYV" );
        outln( "    - single raw from in YUV 4:2:2 format at specified resolution");
        outln( "    - should be converted to another format (jpg/png) before attempting to view" );
        outln( "    - should be converted to another format (jpg) before attempting to write to multi-frame video file" );
        outln( "" );
        outln( "H.264 or H.265" );
        outln( "    - single frame of encoded (h.264 or h.265 video");
        outln( "    - can be saved (one or more frames) to MP4 file but may not be in correct format without some fixing up by ffmpeg");
        outln( "    - this line will fix up the video (v4l2cam uses 30 fps as a default) : ffmpeg -r 30 -i ../test.mp4 ../test2.mp4");
        outln( "    - in fact this line will do it all in one step : ./v4l2cam -c 7 -d 2 -t 5 | ffmpeg -r 30 -i pipe: ../test3.mp4");
    #endif

    outln( "" );
    outln( "...discovery complete" );

}

void listUserControls( std::string deviceID )
{
    // create the camera object
    #ifdef __linux__
        std::vector< LinuxCamera *> camList;
        camList = LinuxCamera::discoverCameras();
        LinuxCamera * tmp = camList[std::stoi(deviceID)];
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * tmp = camList[std::stoi(deviceID)];
    #endif

    outln( "" );
    outln( "------------------------------");
    outln( "User Control(s) for " + tmp->getDevName() + " : " + tmp->getUserName() );

    if( verbose ) tmp->setLogMode( v4l2cam_logging_mode::logToStdOut );
    else tmp->setLogMode( v4l2cam_logging_mode::logOff );

    if( tmp )
    {
        if( tmp->open() )
        {
            for( const auto &x : tmp->getControls() )
            {
                struct v4l2cam_control ct = x.second;
                outln( std::to_string(x.first) + "\t[" 
                                + ct.typeStr + "] "
                                + "\tmin = " + std::to_string(ct.min)
                                + " max = " + std::to_string(ct.max)
                                + " step = " + std::to_string(ct.step)
                                + "\t: " + ct.name

                                );
                #ifdef __APPLE__
                    if( ct.type == v4l2cam_control_type::v4l2_menu )
                    {
                #elif __linux__
                    if( ct.type == V4L2_CTRL_TYPE_MENU )
                    {
                #endif
                        std::string menStr = "\t\t[menu]\t" + std::to_string(ct.menuItems.size()) + " item(s) : \n\t\t\t";
                        for( const auto &y : ct.menuItems ) menStr += "[" + std::to_string(y.first) + "] " + y.second + "\n\t\t\t";
                        outln( menStr );
                    }
            }
            outln( "" );
            outln( "..." + std::to_string(tmp->getControls().size()) + " user controls supported" );

            tmp->close();
        }
    }
    // delete all the camera object now that we are done with them
    for( const auto &x : camList ) delete x;

    outln( "");
    outln( "...discovery complete" );

}