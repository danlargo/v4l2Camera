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
#endif


// encourage the low level objects to spew out information
bool verbose = false;
// run totally silent so that v4l2cam can be used in scripts
bool silentMode = false;

int main( int argc, char** argv )
{
    // check for command line arguments and then bail
    if( argc == 1 ) return printBasicHelp();

    // parse the input command line
    std::map<std::string, std::string> cmdLine = parseCmdLine( argc, argv );

    // show welcome message
    if( !silentMode ) outinfo( "Welcome to " + std::string(argv[0]) + "   ...using V4l2Camera " + V4l2Camera::getVersionString() + " " + V4l2Camera::getCodeName() );
    else 
    {
        outinfo( "Welcome to " + std::string(argv[0]) + "   ...using V4l2Camera " + V4l2Camera::getVersionString() + " " + V4l2Camera::getCodeName() );
        outinfo( "...running in silent mode" );
    }

    //
    // now process the commands, only one command at a time
    //

#ifdef _WIN32
    // Windows specific initialization
    WinCamera::initMF();
#endif

    // always print the version, if requested
	if (cmdLine["v"] == "1") printVersionInfo();

    // Exclusive Commands, execute and return

    // show example commands
	if (cmdLine["x"] == "1") { printExamples(); return 0; }

    // list of USB Cameras in the system
	if (cmdLine["l"] == "1") { listUSBCameras( false ); return 0; }
    
    // list of streaming capable USB Cameras in the system
	if (cmdLine["L"] == "1") { listUSBCameras( true ); return 0; }

	// list all devices
	if (cmdLine["i"] == "1") { listAllDevices(); return 0; }


    // Test Mode, requires device indicator
    if( cmdLine["T"] == "1" )
    {
        if( cmdLine["d"].length() > 0 ) runTimingTest( cmdLine["d"] ) ;
        else outwarn( "Must provide a device number to run timing tests : -d [0..63]" );
    }

    // Video Modes, requires device indicator
    if( cmdLine["m"] == "1" )
    {
        if( cmdLine["d"].length() > 0 ) listVideoModes( cmdLine["d"] ) ;
        else outwarn( "Must provide a device number to list video modes : -d [0..63]" );
    }

    // User Controls, requires device indicator
    else if( cmdLine["u"] == "1")
    {
        // make sure there is a device specified
        if (cmdLine["d"].length() > 0) listUserControls(cmdLine["d"]);
        else outwarn("Must provide a device number to list user controls : -d [0..63]");
    }
            
    // Try to Grab Meta Data, requires device indicator
    else if( cmdLine["M"] == "1")
    {
        // make sure there is a device specified
        if (cmdLine["d"].length() > 0) fetchMetaData(cmdLine["d"]);
        else outwarn("Must provide a device number to grab meta data : -d [0..63]");
    }
        
    //Get Video Format and FPS rate, must have a device as second parameter
    else if( cmdLine["?"].length() > 0) getVideoFormat(cmdLine["?"]);

    // Grab an Image, will set video mode and frame rate first if requested
    else if( cmdLine["g"] == "1" )
    {
        // make sure there is a device specified
        if( cmdLine["d"].length() > 0 ) captureFrame(cmdLine["d"], cmdLine["o"], cmdLine["f"]);
        else outwarn("Must provide a device number to grab an image : -d [0..63]");
    }
                    
    // Capture Video, will set video mode and frame rate first if requested
    else if( cmdLine["c"] == "1")
    {
        // make sure there is a device specified
        if (cmdLine["d"].length() > 0) captureFrames(cmdLine["d"], cmdLine["t"], cmdLine["o"], cmdLine["H"]);
        else outwarn("Must provide a device number to start video capture : -d [0..63]");
    }
                        
    // Get Control Value, must have a device and a control number
    else if( cmdLine["r"].length() > 0)
    {
        // make sure there is a device specified
        if (cmdLine["d"].length() > 0) getControlValue(cmdLine["d"], cmdLine["r"]);
        else outerr("Must provide a device number to get a control : -d [0..63]");
    }
                            
    // Set Control Value, must have a device and a control number and a new value
    else if( cmdLine["s"].length() > 0)
    {
        // make sure there is a device specified
        if (cmdLine["d"].length() > 0) setControlValue(cmdLine["d"], cmdLine["s"], cmdLine["s2"]);
        else outwarn("Must provide a device number to set a control : -d [0..63]");
    }

    // Set Frame Format, requires device indicator and video mode, optional frame rate
    else if( cmdLine["F"].length() > 0 )
    {
        // make sure there is a device specified
        if (cmdLine["d"].length() > 0) 
        {
            // if frame rate also set, do them both at the same time
            if( cmdLine["R"].length() > 0 ) setVideoFormat(cmdLine["d"], cmdLine["F"], cmdLine["R"]);
            else setVideoFormat(cmdLine["d"], cmdLine["F"], "30");
        }
        else outwarn("Must provide a device number to set video mode : -d [0..63]");
    }

    // Set Frame Rate, requires device indicator and rate (> 0 and < 60)
    else if( cmdLine["R"].length() > 0 )
    {
        // make sure there is a device specified
        if (cmdLine["d"].length() > 0) setFrameRate(cmdLine["d"], cmdLine["R"]);
        else outwarn("Must provide a device number to set frame rate : -d [0..63]");
    }

        
#ifdef _WIN32
    // Windows specific shutdown
    WinCamera::shutdownMF();
#endif

    return 0;
}


std::map<std::string, std::string> parseCmdLine( int argc, char** argv )
{
    std::map<std::string, std::string> cmdLine;

    //
    // parse the command options, process them once we know everything that was requested
    //
    int i = 1;
    while( i < argc )
    {
        std::string argS = argv[i++];

        // fouund help request, print the help message nd ignore everything else
        if( argS == "-h" ) 
        {
            printBasicHelp();
            cmdLine.clear();
            return cmdLine;
        }

        // found silent flag, turn it on ASAP, supress verbose flag
        if( argS == "-S" ) { silentMode = true; verbose = false; continue; }

        // found verbose flag, turn it on ASAP
        if( argS == "-V" ) { if( !silentMode ) verbose = true; continue; }

        // Version request
        if( argS == "-v" ) { cmdLine["v"] = "1"; continue; }

        // Discover USB Cameras
        if( argS == "-l" ) { cmdLine["l"] = "1"; continue; }
        
        // Discover USB Cameras
        if( argS == "-L" ) { cmdLine["L"] = "1"; continue; }

        // Add header to output H264 frames
        if( argS == "-H" ) { cmdLine["H"] = "1"; continue; }

        // Identify all Devices query
        if( argS == "-i" ) { cmdLine["i"] = "1"; continue; }

        // List all video mdoes
        if( argS == "-m" ) { cmdLine["m"] = "1"; continue; }

        // List all user controls
        if( argS == "-u" ) { cmdLine["u"] = "1"; continue; }
        
        // Run timing tests at current resolution and frame rate
        if( argS == "-T" ) { cmdLine["T"] = "1"; continue; }

        // Grab an Image
        if( argS == "-g" ) { cmdLine["g"] = "1"; continue; }
        
        // Capture Video
        if( argS == "-c" ) { cmdLine["c"] = "1"; continue; }

        // Get Meta Data
        if( argS == "-M" ) { cmdLine["M"] = "1"; continue; }
        
        // Show some sample commands
        if( argS == "-x" ) { cmdLine["x"] = "1"; continue; }
        
        // Specify device number for operation, specify device number in second parameter, between 0 and 63
        if( argS == "-d" )
        {
            if( (i < argc) && (is_number(argv[i])) ) { cmdLine["d"] = argv[i++]; continue; }
            else
            {
                outerr( "Invalid attribute for Device Number [-d]" );
                printBasicHelp();
                cmdLine.clear();
                return cmdLine;
            }
        }

        // Specify device number for quering current format, specify device number in second parameter, between 0 and 63
        if( argS == "-?" )
        {
            if( (i < argc) && (is_number(argv[i])) ) { cmdLine["?"] = argv[i++]; continue; }
            else
            {
                outerr( "Invalid attribute for Device Number [-?]" );
                printBasicHelp();
                cmdLine.clear();
                return cmdLine;
            }
        }


        // Retrieve Control Value - specify control number in second parameter
        if( argS == "-r" )
        {
            if( (i < argc) && (is_number(argv[i])) ) { cmdLine["r"] = argv[i++]; continue; }
            else
            {
                outerr( "Invalid attribute for Retrieve Control Value [-r]" );
                printBasicHelp();
                cmdLine.clear();
                return cmdLine;
            }
        }

        // Set Frame Format - specify mode number in second parameter
        if( argS == "-F" )
        {
            if( (i < argc) && (is_number(argv[i])) ) { cmdLine["F"] = argv[i++]; continue; }
            else
            {
                outerr( "Invalid attribute for Set Video mode [-F]" );
                printBasicHelp();
                cmdLine.clear();
                return cmdLine;
            }
        }

        // Set Frame Rate, specify mode number in second parameter
        if( argS == "-R" )
        {
            if( (i < argc) && (is_number(argv[i])) ) { cmdLine["R"] = argv[i++]; continue; }
            else
            {
                outerr( "Invalid attribute for Frame Rate [-p]" );
                printBasicHelp();
                cmdLine.clear();
                return cmdLine;
            }
        }

        // Set Control Value - specify control number in second parameter and new value in 3rd parameter
        if( argS == "-s" )
        {
            if( (i < argc) && (is_number(argv[i])) ) 
            {
                cmdLine["s"] = argv[i++];
                // check for second parameter
                if( (i < argc) && (is_number(argv[i])) ) { cmdLine["s2"] = argv[i++]; continue; }
                else
                {
                    outerr( "Set Control Value must have 2 attributes [-s]" );
                    printBasicHelp();
                    cmdLine.clear();
                    return cmdLine;
                }
            } else
            {
                outerr( "Invalid attribute for Set Control Value [-s]" );
                printBasicHelp();
                cmdLine.clear();
                return cmdLine;
            }
        }

        // Set Time duration  - specify time (seconds) in second parameter
        if( argS == "-t" )
        {
            if( (i < argc) && (is_number(argv[i])) ) { cmdLine["t"] = argv[i++]; continue; }
            else
            {
                outerr( "Invalid attribute for Time duration [-t]" );
                printBasicHelp();
                cmdLine.clear();
                return cmdLine;
            }
        }

        // Output File name, default to STDOUT if not set, second parameter is filename
        if( argS == "-o" )
        {

            if( (i < argc) ) { cmdLine["o"] = argv[i++]; continue; }
            else
            {
                outerr( "Invalid attribute for File name [-o]" );
                printBasicHelp();
                cmdLine.clear();
                return cmdLine;
            }
        }

        // Output Image format, defaults to raw if not specified, second parameter is fmt (jpg, bmp, raw)
        if (argS == "-f")
        {

            if ((i < argc)) { cmdLine["f"] = argv[i++]; continue; }
            else
            {
                outerr("Invalid attribute for Imaeg format [-f]");
                printBasicHelp();
                cmdLine.clear();
                return cmdLine;
            }
        }

        // if we made it here then the command is invalid
        outerr( "Invalid command [" + argS + "]" );
        printBasicHelp();
        cmdLine.clear();
        return cmdLine;
    }

    return cmdLine;
}
