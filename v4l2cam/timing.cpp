#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <chrono>
#include <array>

#include "defines.h"
#include "image_utils/image_utils.h"

#ifdef __linux__
    #include <unistd.h>
    #include "linuxcamera.h"
#elif __APPLE__
    #include <unistd.h>
    #include "macos/maccamera.h"
    #include "macos/v4l2cam_defs.h"
#elif _WIN32
    #include "wincamera.h"
#endif


void runTimingTest( std::string deviceID )
{
    int num_fetches = 100;
    int cur_fetch = 100;

    int good_frames = 0;
    int bad_frames = 0;

    long long total_time = 0;
    int total_bytes = 0;

    std::vector<int> frame_times;

    // create the camera object
    v4l2cam_logging_mode t = v4l2cam_logging_mode::logOff;
    if (verbose) t = v4l2cam_logging_mode::logToStdOut;

    #ifdef __linux__
        std::vector< LinuxCamera *> camList;
        camList = LinuxCamera::discoverCameras(t);
        LinuxCamera * cam = camList[std::stoi(deviceID)];
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * cam = camList[std::stoi(deviceID)];
    #elif _WIN32
        std::vector<WinCamera *> camList;
        camList = WinCamera::discoverCameras(t);
        WinCamera * cam = camList[std::stoi(deviceID)];
    #endif

    // initiate image (one frame) capture
    outinfo( "Using " + cam->getDevName() + " : " + cam->getUserName() + " for timing test");

    if( verbose ) cam->setLogMode( v4l2cam_logging_mode::logToStdOut );
    else cam->setLogMode( v4l2cam_logging_mode::logOff );

    if( cam && cam->open() )
    {
        // display the current video mode and frame rate
        struct v4l2cam_video_mode * data = cam->getFrameFormat();
        int data2 = -1;
        if( data )
        {
            // grab the frame format
            data2 = cam->getFrameRate();
            outinfo( "   ...using format : " + data->format_str + 
                    " @ " + std::to_string(data->width) + " x " + std::to_string(data->height) + 
                    " : " + std::to_string(data2) + " fps" );

        } else outwarn( "Failed to fetch current video format for : " + cam->getDevName() + " " + cam->getUserName() );

        // initialize the camera
        if( cam->init( v4l2cam_fetch_mode::userPtrMode ) )
        {
            // get start time
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

            while( cur_fetch > 0 )
            {
                bool last_frame = (cur_fetch > 1) ? false : true;
                cur_fetch--;

                // grab a single frame
                struct v4l2cam_image_buffer* inB = cam->fetch(last_frame);

                // get fetch time
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                std::chrono::microseconds time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                start = end;

                total_time += time_span.count();
                frame_times.push_back(time_span.count());

                if( inB && inB->buffer )
                {
                    // check for invalid JPG file (if MJPG imag format)
                    if( "MJPG" == data->format_str )
                    {
                        // check the header, should be 0xFF 0xD8 0xFF 
                        if( (inB->buffer[0] == 0xff) && (inB->buffer[1] == 0xd8) && ((int)inB->buffer[2] == 0xff) ) good_frames++;
                        else bad_frames++;
                    } else good_frames++;

                    // count the bytes
                    total_bytes += inB->length;

                    // delete the buffers
                    if( inB->buffer ) delete inB->buffer;
                    if( inB ) delete inB;

                } else bad_frames++;

            }

            // close the camera
            cam->close();

            std::locale loc("");
            std::cout.imbue(loc);

            // output the results
            outinfo( "" );
            outinfo( "Timing Test Results for : " + cam->getDevName() + " " + cam->getUserName() );
            outinfo( "   ...good frames : " + std::to_string(good_frames) );
            outinfo( "   ...bad frames : " + std::to_string(bad_frames) );
            outinfo( "   ...total bytes : " + std::to_string(total_bytes) );
            outinfo( "   ...average bytes per frame : " + std::to_string(total_bytes / good_frames) );
            outinfo( "   ...average frame time : " + std::to_string((total_time / good_frames)/1000) + " ms" );
            outinfo( "" );
            int calc_fps = 1000000 / (total_time / good_frames);
            if( calc_fps < (.8*data2) ) outwarn( "   ...average frame rate : " + std::to_string(calc_fps) + " fps, versus " + std::to_string(data2) + " fps" );
            else outinfo( "   ...average frame rate : " + std::to_string(calc_fps) + " fps, requested " + std::to_string(data2) + " fps" );

        } else outwarn( "Failed to initilize fetch mode for : " + cam->getDevName() + " " + cam->getUserName() );

        // close the camera
        cam->close();

    } else outerr( "Failed to create/open camera : + deviceID" );

    // delete the camera object
    for( const auto &x : camList ) delete x;

}