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
        outln( "Control value from : " + tmp->getDevName() + " : " + tmp->getUserName() );
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
            if( silentMode ) std::cout << getVal << std::endl;
            else 
            {
                outln( "" );
                outln( "Control " + cntrl.name + " [" + cntrlID + "] = " + std::to_string(getVal) );
                outln( "   ...min : " + std::to_string(cntrl.min) + "  max : " + std::to_string(cntrl.max) + " step : " + std::to_string(cntrl.step) );
            }
        } catch(...)
        {
            outerr( "Control ID " + cntrlID + " not found or invalid" );
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
                if( ret ) std::cout << "1" << std::endl;
                else std::cout << "0" << std::endl;
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
        }
    }

    if( !silentMode )
    {
        outln( "");
        outln( "...operation complete" );
    }

    return ret;
}