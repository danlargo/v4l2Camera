#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <cstring>
#include <string>
#include <sstream>
#include <chrono>

#include "defines.h"

#ifdef __linux__
    #include "linuxcamera.h"
#elif __APPLE__
    #include "maccamera.h"
#endif

#include <unistd.h>

int main( int argc, char** argv )
{
    #ifdef __APPLE__
        // init the handler library for the MACCamera class
        MACCamera::initAPI();
    #endif

    // check for command line arguments and then bail
    if( argc == 1 ) 
    {
        #ifdef __APPLE__
            MACCamera::closeAPI();
        #endif
        return printBasicHelp();
    }

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
                    if( cmdLine["d"].length() > 0 ) grabImage( cmdLine["d"], cmdLine["g"], cmdLine["o"] );
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

    // close down the handler library for the MACCamera class
    #ifdef __APPLE__
        MACCamera::closeAPI();
    #endif

    return 0;
}

// Functional demonstration functions
void listUSBCameras()
{
    outinfo( "" );
    outinfo( "USB camera discovery starting..." );

    #ifdef __linux__
        V4l2Camera * tmp = nullptr;
        std::map<int, V4l2Camera *> camList;
        camList = V4l2Camera::discoverCameras();
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
    #endif

    // output the collected information
    outln( "" );
    outln( "Detected " + std::to_string(camList.size()) + " USB camera(s)" );
    outln( "" );

    for( int i=0;i<camList.size();i++ )
    {
        if( verbose ) camList[i]->setLogMode( logging_mode::logToStdOut );
        else camList[i]->setLogMode( logging_mode::logOff );

        if( camList[i]->open() )
        {
            outln( "[" + std::to_string(i) + "] "
                    + camList[i]->getDevName() + " : " + camList[i]->getUserName() + ", " 
                    + std::to_string(camList[i]->getVideoModes().size()) + " video modes, "
                    + std::to_string(camList[i]->getControls().size()) + " user controls"
                );
        }
        // close and delete he camera now that we are done with it
        camList[i]->close();
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
            V4l2Camera * tmpC = new V4l2Camera(nam);
            if( verbose ) tmpC->setLogMode( logging_mode::logToStdOut );

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

    #elif __APPLE__
        outln( "*** Not supported on MACOS" );
        outln( "" );
    #endif
}

void listVideoModes( std::string deviceID )
{
    // create the camera object
    #ifdef __linux__
        V4l2Camera * tmp = new V4l2Camera( "/dev/video" + deviceID );
    #elif __APPLE__
        std::vector< MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * tmp = camList[std::stoi(deviceID)];
    #endif

    outln( "" );
    outln( "------------------------------");
    outln( "Video Mode(s) for camera [" + deviceID + "] " + tmp->getDevName() + " : " + tmp->getUserName() );

    if( verbose ) tmp->setLogMode( logging_mode::logToStdOut );
    else tmp->setLogMode( logging_mode::logOff );

    if( tmp )
    {
        if( tmp->open() )
        {
            std::vector<video_mode> modes = tmp->getVideoModes();
            outln( "" );
            outln( "...Found " + std::to_string(modes.size()) + " video modes, details following..." );
            outln( "" );

            int offset = 0;
            for( auto x : tmp->getVideoModes() )
            {
                struct video_mode vm = x;
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
        V4l2Camera * tmp = new V4l2Camera( "/dev/video" + deviceID );
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * tmp = camList[std::stoi(deviceID)];
    #endif

    outln( "" );
    outln( "------------------------------");
    outln( "User Control(s) for " + tmp->getDevName() + " : " + tmp->getUserName() );

    if( verbose ) tmp->setLogMode( logging_mode::logToStdOut );

    if( tmp )
    {
        if( tmp->open() )
        {
            for( const auto &x : tmp->getControls() )
            {
                struct user_control ct = x.second;
                outln( std::to_string(x.first) + "\t[" 
                                + ct.typeStr + "] "
                                + "\tmin = " + std::to_string(ct.min)
                                + " max = " + std::to_string(ct.max)
                                + " step = " + std::to_string(ct.step)
                                + "\t:" + ct.name

                                );
                #ifdef __linux__
                    if( ct.type == V4L2_CTRL_TYPE_MENU )
                    {
                        std::string menStr = "     menu items are :";
                        for( const auto &y : ct.menuItems ) menStr += " [" + std::to_string(y.first) + "]" + y.second;
                        outln( menStr );
                    }
                #endif
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

void grabImage( std::string deviceID, std::string videoMode, std::string fileName )
{
    bool sendToStdout = true;
    std::ofstream outFile;

    // check if filename is specified
    if( fileName.length() > 0 ) sendToStdout = false;
    else outerr( "No filename specified, writing output data (image frame) to STDOUT");

    // open the output file
    if( !sendToStdout ) 
    {
        outFile.open( fileName, std::ios::trunc | std::ios::binary );
        if( !outFile.is_open() ) 
        {
            outerr( "Failed to open output file : " + fileName + " - " + strerror(errno) );
            return;
        }
    }

    // create the camera object
    #ifdef __linux__
        V4l2Camera * cam = new V4l2Camera( "/dev/video" + deviceID );
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * cam = camList[std::stoi(deviceID)];
    #endif

    // initiate image (one frame) capture
    outerr( "Using " + cam->getDevName() + " : " + cam->getUserName() + " for image capture");

    if( verbose ) cam->setLogMode( logging_mode::logToStdOut );

    if( cam && cam->open() )
    {
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
            if( cam->init( fetch_mode::userPtrMode ) )
            {
                // grab a single frame
                struct image_buffer * inB = cam->fetch(true);
                if( inB && inB->buffer )
                {
                    // check for invalid JPG file (if Motion-JPEG selected)
                    if( "Motion-JPEG" == vm.format_str )
                    {
                        // check the header, should be 0xFF 0xD8 0xFF 
                        if( (inB->buffer[0] == 0xff) && (inB->buffer[1] == 0xd8) && ((int)inB->buffer[2] == 0xff) ) 
                        {} else
                        {
                            outerr( "Invalid JPG header, should be [FFD8FF], it is actually [" 
                                        + makeHexString(inB->buffer, 3, true )
                                        + "] : buflen is " 
                                        + std::to_string(inB->length) );
                            outerr( "...repeating fetch" );

                            // retry the fetch (10 times at most)
                            int tries = 0;
                            bool goodFrame = false;
                            while( tries < 10 )
                            {
                                // re have to re-initialize as we didn't queue up a buffer last time
                                cam->close();
                                if( cam->open() && cam->setFrameFormat( vm ) && cam->init( fetch_mode::userPtrMode ) )
                                {
                                    // free the previous buffers
                                    delete inB->buffer;
                                    delete inB;

                                    struct image_buffer * inB = cam->fetch(true);
                                    if( inB && inB->buffer )
                                    {
                                        if( (inB->buffer[0] == 0xff) && (inB->buffer[1] == 0xd8) && ((int)inB->buffer[2] == 0xff) ) 
                                        {
                                            goodFrame = true;
                                            break;
                                        }
                                        // do it again
                                        outerr( "...and again" );
                                        tries ++;
                                    } else {
                                        outerr( "...re-fetch failed, giving up" );
                                        break;
                                    }
                                } else {
                                    outerr( "...failed to re-init the camera, giving up" );
                                    break;
                                }
                            }
                            if( goodFrame ) outerr( "...got good frame" );
                        }
                    }

                    // write the buffer out to the file
                    if( sendToStdout ) std::cout.write( (char*)inB->buffer, inB->length );
                    else outFile.write( (char *)inB->buffer, inB->length );

                    // delete the returned data
                    delete inB->buffer;
                    delete inB;

                } else outerr( "Nothing returned from fetch call for : " + cam->getDevName() + " " + cam->getUserName() );
            } else outerr( "Failed to initilize fetch mode for : " + cam->getDevName() + " " + cam->getUserName() );
        } else outerr( "Failed to set video mode for : " + cam->getDevName() + " " + cam->getUserName() );

        // close the camera
        cam->close();

    } else outerr( "Failed to create/open camera : + deviceID" );

    // close the file
    if( !sendToStdout ) outFile.close();
    else std::cout.flush();

}

void captureVideo( std::string deviceID, std::string videoMode, std::string timeDuration, std::string fileName )
{
    bool sendToStdout = true;
    std::ofstream outFile;
    
    int timeToCapture = 10;
    int fpsVideo = 30;      // let's try to capture 30 FPS
    int framesToCapture;

    #ifdef __linux__
        V4l2Camera * cam = new V4l2Camera( "/dev/video" + deviceID );
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * cam = camList[std::stoi(deviceID)];
    #endif

    // check if filename is specified
    if( fileName.length() > 0 ) 
    {
        outerr( "Using " + cam->getDevName() + " : " + cam->getUserName() +  " for video capture" );
        sendToStdout = false;
    }
    else outerr( "No filename specified, writing output data (video frames) to STDOUT");

    // check the time duration
    if( timeDuration.length() > 0 ) timeToCapture = std::stoi( timeDuration );
    else outerr( "No time duration specified");

    // check for max duration
    if( timeToCapture > 60 )
    {
        outerr( "Max capture time is 60 secs" );
        timeToCapture = 60;
    }
    
    // initiate video (multiple frame) capture
    framesToCapture = timeToCapture * fpsVideo;
    outerr( "Video capture duration is : " + std::to_string(timeToCapture) + " seconds, " + std::to_string(framesToCapture) + " frames" );

    // open the output file
    if( !sendToStdout ) 
    {
        outFile.open( fileName, std::ios::trunc | std::ios::binary );
        if( !outFile.is_open() ) 
        {
            outerr( "Failed to open output file : " + fileName + " - " + strerror(errno) );
            return;
        }
    }

    if( verbose ) cam->setLogMode( logging_mode::logToStdOut );

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
            outerr( "Failed to find matching video mode - operation aborted ");
            return; 
        }
        // set the video mode
        if( cam->setFrameFormat( vm ) )
        {
            outerr( "Set image mode to : " + vm.format_str + " @ " + std::to_string(vm.width) + "x" + std::to_string(vm.height) );
            // initialize the camera
            if( cam->init( fetch_mode::userPtrMode ) )
            {
                // set up the fpsVideo timers
                std::chrono::steady_clock::time_point start;
                typedef std::chrono::duration<int,std::milli> millisec_t ;
                millisec_t delta;

                start = std::chrono::steady_clock::now();

                // grab a a bunch of frames
                while( framesToCapture > 0 )
                {
                    bool goodFrame = false;
                    struct image_buffer * inB = cam->fetch(framesToCapture == 1);

                    // check the return buffers
                    if( inB && inB->buffer )
                    {
                        // check for invalid JPG file (if MOtion-JPEG selected)
                        if( "Motion-JPEG" == vm.format_str )
                        {
                            // check the header, should be 0xFF 0xD8 0xFF 
                            if( (inB->buffer[0] == 0xff) && (inB->buffer[1] == 0xd8) && ((int)inB->buffer[2] == 0xff) ) goodFrame = true;

                        } else goodFrame = true;

                        if( goodFrame )
                        {
                            // write the buffer out to the file
                            if( sendToStdout ) std::cout.write( (char*)inB->buffer, inB->length );
                            else outFile.write( (char *)inB->buffer, inB->length );
                        }

                        // delete the returned data
                        delete inB->buffer;
                        delete inB;

                    } else outerr( "Nothing returned from fetch call for : /dev/video" + deviceID );

                    framesToCapture--;

                    // make sure we are waiting 1000/fpsVideo milliseconds each look
                    delta = std::chrono::duration_cast<millisec_t> (std::chrono::steady_clock::now() - start );
                    while( delta < (std::chrono::milliseconds)(1000/fpsVideo) )
                    {
                        delta = std::chrono::duration_cast<millisec_t> (std::chrono::steady_clock::now() - start );
                        usleep( 1000 );
                    }
                    start = std::chrono::steady_clock::now();

                }
            } else outerr( "Failed to initilize fetch mode for : " + cam->getDevName() + " " + cam->getUserName()  );
        } else outerr( "Failed to set video mode for : " + cam->getDevName() + " " + cam->getUserName()  );

        // close the camera
        cam->close();

    } else outerr( "Failed to create/open camera " + deviceID );

    // close the file
    if( sendToStdout ) std::cout.flush();
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

void outln( std::string line )
{
    if( !silentMode ) std::cout << "   " << line << std::endl;
}

void outerr( std::string line )
{
    // always print the error messages
    std::cerr << "[\x1b[1;31mwarning\x1b[0m] " << line << std::endl;
}

void outinfo( std::string line )
{
    if( !silentMode ) if( verbose ) std::cout << "[\x1b[1;33minfo\x1b[0m] " << line << std::endl;
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

std::string makeHexString( unsigned char * buf, int len, bool makeCaps )
{
    std::stringstream tmp;

    int bufOff = 0;

    while( len > 0 )
    {
        if( makeCaps ) tmp << std::uppercase;
        tmp << std::setfill('0') << std::setw(2) << std::hex << (int)buf[bufOff++];
        len--;
    }

    return tmp.str();
}