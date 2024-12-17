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
    if( !silentMode ) std::cerr << std::endl << "Welcome to " << argv[0] 
                                << "   ...using V4l2Camera " << V4l2Camera::getVersionString() << " " << V4l2Camera::getCodeName()
                                << std::endl << std::endl;
    //
    // now process the commands, some depend on others, some can be done in sequence 
    // ex. -l -a can both be completed in single cmd line request
    //

    // always print the version info if asked
    if( cmdLine["v"] == "1" ) printVersionInfo();

    // show example commands
    if( cmdLine["x"] == "1" ) printExamples();

    else {

        // Video Mode - should be done alone, skip all other commands
        if( cmdLine["m"] == "1" )
        {
            // make sure there is a device specified
            if( cmdLine["d"].length() > 0 ) listVideoModes( cmdLine["d"] ) ;
            else outerr( "Must provide a device number to list video modes : -d [0..63]" );
            
        } else {

            // User Controls - should be done alone, skip all other commands
            if( cmdLine["u"] == "1" )
            {
                // make sure there is a device specified
                if( cmdLine["d"].length() > 0 ) listUserControls( cmdLine["d"] ) ;
                else outerr( "Must provide a device number to list user controls : -d [0..63]" );
                
            } else {

                // Grab an Image - should be done alone, skip all other commands
                if( cmdLine["g"].length() > 0 )
                {
                    outerr( "Capturing image, all other commands will be ignored" );

                    // make sure there is a device specified
                    if( cmdLine["d"].length() > 0 ) captureImage( cmdLine["d"], cmdLine["g"], cmdLine["o"] );
                    else outerr( "Must provide a device number to grab an image : -d [0..63]" );
                    
                } else {

                    // Capture Video - should be done alone, skip all other commands
                    if( cmdLine["c"].length() > 0 )
                    {
                        outerr( "Starting video capture, all other commands will be ignored" );
                        
                        // make sure there is a device specified
                        if( cmdLine["d"].length() > 0 ) captureVideo( cmdLine["d"], cmdLine["c"], cmdLine["t"], cmdLine["o"] ) ;
                        else outerr( "Must provide a device number to start video capture : -d [0..63]" );
                        
                    } else {

                        if( cmdLine["l"] == "1" ) listUSBCameras();

                        if( cmdLine["i"] == "1" ) listAllDevices();
                    }
                }
            }
        }
    }

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

        // Identify all Devices query
        if( argS == "-i" ) { cmdLine["i"] = "1"; continue; }

        // List all video mdoes
        if( argS == "-m" ) { cmdLine["m"] = "1"; continue; }

        // List all user controls
        if( argS == "-u" ) { cmdLine["u"] = "1"; continue; }
        
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

        // GRab an Image - specify video mode in second parameter
        if( argS == "-g" )
        {
            if( (i < argc) && (is_number(argv[i])) ) { cmdLine["g"] = argv[i++]; continue; }
            else
            {
                outerr( "Invalid attribute for Grab Image [-g]" );
                printBasicHelp();
                cmdLine.clear();
                return cmdLine;
            }
        }

        // Capture Video - specify video mode in second parameter
        if( argS == "-c" )
        {
            if( (i < argc) && (is_number(argv[i])) ) { cmdLine["c"] = argv[i++]; continue; }
            else
            {
                outerr( "Invalid attribute for Capture Video [-c]" );
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

        // if we made it here then the command is invalid
        outerr( "Invalid command [" + argS + "]" );
        printBasicHelp();
        cmdLine.clear();
        return cmdLine;
    }

    return cmdLine;
}
