#include <iostream>
#include <string>
#include <cstdint>

#include "defines.h"

#ifdef __linux__
    #include <unistd.h>
    #include "linux/linuxcamera.h"
#elif __APPLE__
    #include <unistd.h>
    #include "macos/maccamera.h"
    #include "macos/v4l2cam_defs.h"
#elif _WIN32
    #include "wincamera.h"
#endif

int getControlValue( std::string deviceID, std::string cntrlID )
{
    int ret = INT32_MIN;

    // create the camera 
    v4l2cam_logging_mode t = v4l2cam_logging_mode::logOff;
    if (verbose) t = v4l2cam_logging_mode::logToStdOut;

    #ifdef __linux__
        std::vector< LinuxCamera *> camList;
        camList = LinuxCamera::discoverCameras(t);
        LinuxCamera * tmp = camList[std::stoi(deviceID)];
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * tmp = camList[std::stoi(deviceID)];
    #elif _WIN32
        std::vector<WinCamera *> camList;
        camList = WinCamera::discoverCameras(t);
        WinCamera * tmp = camList[std::stoi(deviceID)];
    #endif

    if( !silentMode )
    {
        outln( "------------------------------");
        outln( "Get Control value from : " + tmp->getDevName() + " : " + tmp->getUserName() );
    }

    if( verbose ) tmp->setLogMode( v4l2cam_logging_mode::logToStdOut );
    else tmp->setLogMode( v4l2cam_logging_mode::logOff );

    if( tmp )
    {
        // make sure the control exists
        struct v4l2cam_control cntrl;
        try
        {
            int cID = std::stoi(cntrlID);
            cntrl = tmp->getOneCntrl( cID );
            int getVal = tmp->getValue( cID, true );
            if( silentMode ) outln( std::to_string(getVal) );
            else 
            {
                outln( "" );
                outln( "Control " + cntrl.name + " [" + cntrlID + "] = " + std::to_string(getVal) );
                outln( "   ...min : " + std::to_string(cntrl.min) + "  max : " + std::to_string(cntrl.max) + " step : " + std::to_string(cntrl.step) );
            }
        } catch(...)
        {
            outerr( "Control ID " + cntrlID + " not found or invalid" );
            if( silentMode ) outln( "-" );          // output error indicator, to trigger scripts, better than returning nothing
        }
    }

    if( !silentMode )
    {
        outln( "");
        outln( "...operation complete" );
    }

    return ret;
}

bool setControlValue( std::string deviceID, std::string cntrlID, std::string newVal )
{
    int ret = false;

    // create the camera object
    v4l2cam_logging_mode t = v4l2cam_logging_mode::logOff;
    if (verbose) t = v4l2cam_logging_mode::logToStdOut;

    #ifdef __linux__
        std::vector< LinuxCamera *> camList;
        camList = LinuxCamera::discoverCameras(t);
        LinuxCamera * tmp = camList[std::stoi(deviceID)];
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * tmp = camList[std::stoi(deviceID)];
    #elif _WIN32       
        std::vector<WinCamera *> camList;
        camList = WinCamera::discoverCameras(t);
        WinCamera * tmp = camList[std::stoi(deviceID)];
    #endif

    if( !silentMode )
    {
        outln( "------------------------------");
        outln( "Setting Control Value for " + tmp->getDevName() + " : " + tmp->getUserName() );
    }

    if( verbose ) tmp->setLogMode( v4l2cam_logging_mode::logToStdOut );
    else tmp->setLogMode( v4l2cam_logging_mode::logOff );

    if( tmp )
    {
        // make sure the control exists
        struct v4l2cam_control cntrl;
        try
        {
            int cID = std::stoi(cntrlID);
            int cVal = std::stoi(newVal);
            cntrl = tmp->getOneCntrl( cID );
            // set the control
            tmp->setValue( cID, cVal, true );
            // confirm the operation
            int con = tmp->getValue(cID, true );
            // set the return
            std::string resStr = "failed";
            if( con == cVal ) 
            {
                ret = true;
                resStr = "success";
            }

            if( silentMode )
            {
                if( ret ) outln( "1" );
                else outln( "0" );
            }
            else
            {
                outln( "" );
                outln( "Control " + cntrl.name + " [" + cntrlID + "] set to : " + newVal );
                outln( "   ...validation read = "  + std::to_string(con) + " : " + resStr );
                outln( "   ...max : " + std::to_string(cntrl.max) + " min : " + std::to_string(cntrl.min) + " step : " + std::to_string(cntrl.step) );
            }
        } catch(...)
        {
            outerr( "Control ID " + cntrlID + " not found or invalid" );
            outln( "-" );          // output error indicator, to trigger scripts, better than returning nothing
        }
    }

    if( !silentMode )
    {
        outln( "");
        outln( "...operation complete" );
    }

    return ret;
}


void getVideoFormat( std::string deviceID )
{
    // create the camera object
    v4l2cam_logging_mode t = v4l2cam_logging_mode::logOff;
    if (verbose) t = v4l2cam_logging_mode::logToStdOut;

    #ifdef __linux__
        std::vector< LinuxCamera *> camList;
        camList = LinuxCamera::discoverCameras(t);
        LinuxCamera * tmp = camList[std::stoi(deviceID)];
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * tmp = camList[std::stoi(deviceID)];
    #elif _WIN32
        std::vector<WinCamera *> camList;
        camList = WinCamera::discoverCameras(t);
        WinCamera * tmp = camList[std::stoi(deviceID)];
    #endif

    if( !silentMode )
    {
        outln( "" );
        outln( "------------------------------");
        outln( "Current Video Format for " + tmp->getDevName() + " : " + tmp->getUserName() );
        outln( "" );
    }

    if( verbose ) tmp->setLogMode( v4l2cam_logging_mode::logToStdOut );
    else tmp->setLogMode( v4l2cam_logging_mode::logOff );

    if( tmp )
    {
        if( tmp->open() )
        {
            struct v4l2cam_video_mode * data = tmp->getFrameFormat();
            if( data )
            {
                // grab the frame format
                int data2 = tmp->getFrameRate();
                if( data2 > -1 )
                {
                    if( !silentMode )
                    {
                        outln( "...format    : " + data->format_str );
                        outln( "...size      : " + std::to_string(data->width) + " x " + std::to_string(data->height) );
                        outln( "...fps       : " + std::to_string(data2) + " fps" );

                    } else std::cout << data->format_str << ", " << data->width << ", " << data->height << ", " << data2 << std::endl;

                } else 
                {
                    outerr( "Failed to fetch frame rate for : " + tmp->getDevName() + " " + tmp->getUserName() );
                    if( silentMode ) outln( "-" );          // output error indicator, to trigger scripts, better than returning nothing
                }
            } else 
            {
                outerr( "Failed to fetch video format for : " + tmp->getDevName() + " " + tmp->getUserName() );
                if( silentMode ) outln( "-" );          // output error indicator, to trigger scripts, better than returning nothing
            }

            tmp->close();
        }
    }
    // delete all the camera object now that we are done with them
    for( const auto &x : camList ) delete x;

    if( !silentMode )
    {
        outln( "");
        outln( "...discovery complete" );
    }
}


void setVideoFormat( std::string deviceID, std::string videoMode, std::string fps  )
{
    // create the camera object
    v4l2cam_logging_mode t = v4l2cam_logging_mode::logOff;
    if (verbose) t = v4l2cam_logging_mode::logToStdOut;

    #ifdef __linux__
        std::vector< LinuxCamera *> camList;
        camList = LinuxCamera::discoverCameras(t);
        LinuxCamera * tmp = camList[std::stoi(deviceID)];
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * tmp = camList[std::stoi(deviceID)];
    #elif _WIN32
        std::vector<WinCamera *> camList;
        camList = WinCamera::discoverCameras(t);
        WinCamera * tmp = camList[std::stoi(deviceID)];
    #endif

    if( !silentMode )
    {
        outln( "" );
        outln( "------------------------------");
        outln( "Setting Video Format / Frame Rate for " + tmp->getDevName() + " : " + tmp->getUserName() );
        outln( "" );
    }

    if( verbose ) tmp->setLogMode( v4l2cam_logging_mode::logToStdOut );
    else tmp->setLogMode( v4l2cam_logging_mode::logOff );

    // grab the requested video mode
    struct v4l2cam_video_mode vm;
    int fpsI = -1;
    try { vm = tmp->getOneVM( std::stoi(videoMode) ); }
    catch(const std::exception& e) 
    { 
        outerr( "Invalid Video Format - operation aborted " + std::string(e.what()) );
        if( silentMode ) outln( "0" );          // indicate FALSE, failure
        return; 
    }

    // grab the frame rate if present
    try { fpsI = std::stoi(fps); } catch(const std::exception& e) { outerr( "Frame rate invalid or not specificed, ignoring " + std::string(e.what()) ); }

    if( tmp )
    {
        if( tmp->open() )
        {
            // set the video mode
            if( !silentMode ) outln(    "...setting video format to : " + vm.format_str +  
                                        " @ " + std::to_string(vm.width) + " x " + std::to_string(vm.height) +
                                        " : " + std::to_string(fpsI) + " fps" );

            if( tmp->setFrameFormat( vm, fpsI ) )
            {
                // display the current video mode and frame rate
                struct v4l2cam_video_mode * data = tmp->getFrameFormat();
                if( data )
                {
                    int val = tmp->getFrameRate();
                    if( !silentMode ) outln(    "...video format set to : " + data->format_str +  
                                                " @ " + std::to_string(data->width) + " x " + std::to_string(data->height) +
                                                " : " + std::to_string(val) + " fps" );
                    else outln( "1" );          // indicate TRUE, success

                } else outerr( "Failed to fetch current video mode" );
            } else outerr( "Failed to set video mode to : " + videoMode );

            tmp->close();

        } else outerr( "Failed to open camera object for device : " + deviceID );
    } else outerr( "Failed to create camera object for device : " + deviceID );
}


void setFrameRate( std::string deviceID, std::string fps  )
{
    // create the camera object
    v4l2cam_logging_mode t = v4l2cam_logging_mode::logOff;
    if (verbose) t = v4l2cam_logging_mode::logToStdOut;

    #ifdef __linux__
        std::vector< LinuxCamera *> camList;
        camList = LinuxCamera::discoverCameras(t);
        LinuxCamera * tmp = camList[std::stoi(deviceID)];
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * tmp = camList[std::stoi(deviceID)];
    #elif _WIN32
        std::vector<WinCamera *> camList;
        camList = WinCamera::discoverCameras(t);
        WinCamera * tmp = camList[std::stoi(deviceID)];
    #endif

    if( !silentMode )
    {
        outln( "" );
        outln( "------------------------------");
        outln( "Setting Frame Rate for " + tmp->getDevName() + " : " + tmp->getUserName() );
        outln( "" );
    }

    if( verbose ) tmp->setLogMode( v4l2cam_logging_mode::logToStdOut );
    else tmp->setLogMode( v4l2cam_logging_mode::logOff );

    // grab the requested video mode
    int fpsI = -1;
    // convert the frame rate
    try 
    { 
        fpsI = std::stoi(fps); 
    } catch(const std::exception& e) 
    { 
        outerr( "Invalid frame rate " + std::string(e.what()) );
        if( silentMode ) outln( "0" );          // indicate FALSE, failure
        return;
    }

    if( tmp )
    {
        if( tmp->open() )
        {
            // try to set the frame rate
            if( tmp->setFrameRate( fpsI ) )
            {
                // verify setting
                int val = tmp->getFrameRate();
                if( !silentMode ) outln( "...frame rate set to : " + std::to_string(val) + " fps" );
                else outln( "1" );          // indicate TRUE, success

            } else outerr( "Failed to set frame rate to : " + fps );

            tmp->close();

        } else outerr( "Failed to open camera object for device : " + deviceID );
    } else outerr( "Failed to create camera object for device : " + deviceID );
}