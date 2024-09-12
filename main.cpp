#include <iostream>
#include <fstream>
#include <map>
#include <cstring>

#include "defines.h"

#include "v4l2camera.h"

int main( int argc, char** argv )
{

    std::cerr   << std::endl
                << "Welcome to " << argv[0] << " app" 
                << "   ...using V4l2Camera " << V4l2Camera::getVersionString() 
                << " " << V4l2Camera::getCodeName()
                << std::endl;

    // check for command line arguments and then bail
    if( argc == 1 ) return printBasicHelp();

    // parse the input command line
    std::map<std::string, std::string> cmdLine = parseCmdLine( argc, argv );

    //
    // now process the commands, some depend on others, some can be done in sequence 
    // ex. -l -a can both be completed in single cmd line request
    //

    // always print the version info if asked
    if( cmdLine["v"] == "1" ) printVersionInfo();

    // Video Mode - should be done alone, skip all other commands
    if( cmdLine["m"] == "1" )
    {
        // make sure there is a device specified
        if( cmdLine["d"].length() > 0 ) listVideoModes( cmdLine["d"] ) ;
        else outerr( "Must provide a device number to list video modes : -d [0..63]" );
        
        return 1;
    }

    // Snap Image - should be done alone, skip all other commands
    if( cmdLine["s"].length() > 0 )
    {
        outerr( "Capturing image, all other commands will be ignored" );

        // make sure there is a device specified
        if( cmdLine["d"].length() > 0 ) snapImage( cmdLine["d"], cmdLine["s"], cmdLine["o"] );
        else outerr( "Must provide a device number to snap an image : -d [0..63]" );
        
        return 1;
    }

    // Capture Video - should be done alone, skip all other commands
    if( cmdLine["c"].length() > 0 )
    {
        outerr( "Starting video capture, all other commands will be ignored" );
        
        // make sure there is a device specified
        if( cmdLine["d"].length() > 0 ) captureVideo( cmdLine["d"], cmdLine["s"], cmdLine["t"], cmdLine["o"] ) ;
        else outerr( "Must provide a device number to start video capture : -d [0..63]" );
        
        return 1;
    }

    if( cmdLine["l"] == "1" ) listUSBCameras();

    if( cmdLine["a"] == "1" ) listAllDevices();

    return 0;
}

// Functional demonstration functions
void listUSBCameras()
{
    std::map<int, V4l2Camera *> camList;

    outinfo( "" );
    outinfo( "USB camera discovery starting..." );

    for( int i=0; i<64; i++ )
    {
        bool keep = false;

        // build the device name
        std::string nam = "/dev/video" + std::to_string(i);

        // create the camera object
        V4l2Camera * tmpC = new V4l2Camera(nam);
        if( verbose ) tmpC->setLogMode( logToStdOut );

        if( tmpC )
        {
            if( tmpC->canOpen() )
            {
                // open the camera so we can query all its capabilities
                if( tmpC->open() )
                {
                    if( tmpC->enumCapabilities() )
                    {
                        if( tmpC->canFetch() )
                        {
                            // have it query its own capabilities
                            tmpC->enumControls();
                            tmpC->enumVideoModes();

                            if( tmpC->getVideoModes().size() > 0 )
                            {
                                // save the camera for display later
                                keep = true;
                                camList[i] = tmpC;
                            } else outinfo( nam + " : zero video modes detected" );
                        } else outinfo( nam + " : does not support video capture" );
                    } else outinfo( nam + " : unable to query capabilities" );
                } else outinfo( nam + " : failed to open device" );
            } else outinfo( nam + " : device indicates unable to open" );
        } else outerr( nam + " : failed to create V4l2Camera object for device" );

        if( !keep ) delete tmpC;
    }

    // output the collected information
    outln( "" );
    outln( "Detected " + std::to_string(camList.size()) + " USB camera(s)" );
    outln( "" );

    for( auto x : camList )
    {
        V4l2Camera * tmp = x.second;

        outln( tmp->getFidName() + " : " + tmp->getUserName() + ", " 
                + std::to_string(tmp->getVideoModes().size()) + " video modes, "
                + std::to_string(tmp->getControls().size()) + " user controls"
            );

        // delete the camera now that we are done with it
        delete tmp;
    }

    outln( "" );
    outln( "...discovery complete" );
}

void listAllDevices()
{
    outln( "" );
    outln( "Devices in range /dev/video[0..63]" );
    outln( "----------------------------------" );

    int numFound = 0;

    for( int i=0; i<64; i++ )
    {

        // build the device name
        std::string nam = "/dev/video" + std::to_string(i);

        // create the camera object
        V4l2Camera * tmpC = new V4l2Camera(nam);
        if( verbose ) tmpC->setLogMode( logToStdOut );

        if( tmpC )
        {
            if( tmpC->canOpen() ) 
            {
                outln( nam + " : can be opened" );
                numFound++;
            }
        } else outerr( nam + " : error !!! - failed to create V4l2Camera object for device" );

        delete tmpC;
    }

    outln( "" );
    outln( "...found : " + std::to_string(numFound) + " devices that can be opened by user" );
    outln( "" );
    outln( "...discovery complete" );
}

void listVideoModes( std::string deviceID )
{
    outln( "" );
    outln( "Video Mode(s) for /dev/video" + deviceID );
    outln( "------------------------------");

    // recreate the camera object
    V4l2Camera * tmp = new V4l2Camera( "/dev/video" + deviceID );
    if( verbose ) tmp->setLogMode( logToStdOut );

    if( tmp )
    {
        if( tmp->open() && tmp->enumVideoModes() )
        {
            for( auto x : tmp->getVideoModes() )
            {
                struct video_mode vm = x.second;
                outln( std::to_string(x.first) + " - " + vm.format_str + " : " + std::to_string(vm.width) + " x " + std::to_string(vm.height) );
            }
            outln( "" );
            outln( "..." + std::to_string(tmp->getVideoModes().size()) + " video modes supported" );
        }
    }
    delete tmp;

    outln( "" );
    outln( "...discovery complete" );

}

void snapImage( std::string deviceID, std::string videoMode, std::string fileName )
{
    bool sendToStdout = true;
    std::ofstream outFile;

    // check if filename is specified
    if( fileName.length() > 0 ) 
    {
        outln("Using /dev/video" + deviceID );
        sendToStdout = false;
    }
    else outerr( "No filename specified, writing output data (image frame) to STDOUT");

    // initiate image (one frame) capture

    // open the output file
    if( !sendToStdout ) 
    {
        outFile.open( fileName );
        if( !outFile.is_open() ) 
        {
            outerr( "Failed to open output file : " + fileName + " - " + strerror(errno) );
            return;
        }
    }

    // create the camera object and try to open if
    V4l2Camera * cam = new V4l2Camera( "/dev/video" + deviceID );
    if( cam && cam->open() )
    {
        // find all the video modes
        cam->enumCapabilities();
        cam->enumVideoModes();

        // grab the requested video mode
        struct video_mode vm;
        try { vm = cam->getOneVM( std::stoi(videoMode) ); }
        catch(const std::exception& e) 
        { 
            outerr( "Failed to set image mode - operation aborted ");
            return; 
        }
        // set the video mode
        if( cam->setFrameFormat( vm ) )
        {
            outerr( "Set image mode to : " + vm.format_str + " @ " + std::to_string(vm.width) + "x" + std::to_string(vm.height) );
            // initialize the camera
            if( cam->init( userPtrMode ) )
            {
                // grab a single frame
                struct image_buffer * inB = cam->fetch(true);
                if( inB )
                {
                    // write the buffer out to the file
                    if( sendToStdout ) std::cout.write( (char*)inB->buffer, inB->length );
                    else outFile.write( (char *)inB->buffer, inB->length );

                    // delete the returned data
                    delete inB->buffer;
                    delete inB;

                } else outerr( "Nothing returned from fetch call for : /dev/video" + deviceID );
            } else outerr( "Failed to initilize fetch mode for : /dev/video" + deviceID );
        } else outerr( "Failed to set video mode for : /dev/video" + deviceID );

        // close the camera
        cam->close();

    } else outerr( "Failed to create/open camera : /dev/video" + deviceID );

    // close the file
    if( sendToStdout ) outFile.close();

}

void captureVideo( std::string deviceID, std::string videoMode, std::string timeDuration, std::string fileName )
{
    bool sendToStdout = true;
    std::ofstream outFile;
    
    int timeToCapture = 10;
    int fpsVideo = 30;      // let's try to capture 30 FPS
    int framesToCapture;

    // check if filename is specified
    if( fileName.length() > 0 ) 
    {
        outln("Using /dev/video" + deviceID + " for video capture" );
        sendToStdout = false;
    }
    else outerr( "No filename specified, writing output data (video frames) to STDOUT");

    // check the time duration
    if( timeDuration.length() > 0 ) timeToCapture = std::stoi( timeDuration );
    else outerr( "No time duration specified");
    
    // initiate video (multiple frame) capture
    framesToCapture = timeToCapture * fpsVideo;
    outerr( "Video capture duration is : " + std::to_string(timeToCapture) );

    // open the output file
    if( !sendToStdout ) 
    {
        outFile.open( fileName );
        if( !outFile.is_open() ) 
        {
            outerr( "Failed to open output file : " + fileName + " - " + strerror(errno) );
            return;
        }
    }

    // write some data
    std::string outData = "Here is some output data";
    if( sendToStdout ) std::cout << outData;
    else outFile.write( outData.c_str(), outData.length() );

    // close the file
    if( sendToStdout ) std::cout << std::endl;
    else outFile.close();

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
    outln( "Usage" );
    outln( "-----" );

    outln( "-h :            this message" );
    outln( "-v :            show version of the V4l2Camera sub system");
    outln( "-V :            operate in verbose mode (lots of information messages)");
    outln( "-l :            list all USB cameras in the /dev/videoX driver space" );
    outln( "-a :            identify all openable devices in /dev/videoX driver space" );
    outln( "-d [0..63] :    select camera /dev/video<number> for operation" );
    outln( "-m :            list all the video modes supported by camera -d [0..63]" );
    outln( "-s [0..??] :    grab an image from camera -d [0..63], using video mode <number>" );
    outln( "-c [0..??] :    capture video from camera -d [0..63], using video mode <number>, for time -t [0..??] seconds, default is 10 seconds" );
    outln( "-o file    :    specify filename for output, will send to stdout if not set" );
    outln( "-t [0..6?? :    specify a time duration for video capture, default is 10 seconds" );
    outln( "" );

    return 1;
}

void outln( std::string line )
{
    std::cout << "   " << line << std::endl;
}

void outerr( std::string line )
{
    std::cerr << std::endl << "[\x1b[1;31mwarning\x1b[0m] " << line << std::endl;
}

void outinfo( std::string line )
{
    if( verbose ) std::cout << "[\x1b[1;33minfo\x1b[0m] " << line << std::endl;
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
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

        // found verbose flag, turn it on ASAP
        if( argS == "-V" ) { verbose = true; continue; }

        // Version request
        if( argS == "-v" ) { cmdLine["v"] = "1"; continue; }

        // USB Camera discovery
        if( argS == "-l" ) { cmdLine["l"] = "1"; continue; }

        // Devices query
        if( argS == "-a" ) { cmdLine["a"] = "1"; continue; }

        // List all video mdoes
        if( argS == "-m" ) { cmdLine["m"] = "1"; continue; }

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

        // Snap Image - specify video mode in second parameter
        if( argS == "-s" )
        {
            if( (i < argc) && (is_number(argv[i])) ) { cmdLine["s"] = argv[i++]; continue; }
            else
            {
                outerr( "Invalid attribute for Snap Image [-s]" );
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