#include <iostream>
#include <map>

#include "defines.h"

#include "v4l2camera.h"

int main( int argc, char** argv )
{
    std::map<std::string, std::string> cmdLine;

    std::cout   << "Welcome to " << argv[0] << " sample application" 
                << "   ...using V4l2Camera " << V4l2Camera::getVersionString() 
                << " " << V4l2Camera::getCodeName()
                << std::endl;

    // check for command line arguments and then bail
    if( argc == 1 ) printBasicHelp();
    else
    {
        // parse the command options, then process them
        int i = 1;
        while( i < argc )
        {
            std::string argS = argv[i++];

            // fouund help request, ignore everything else
            if( argS == "-h" )
            {
                // print the help message nd ignore everything else
                printBasicHelp();
                return 0;
            }

            // found verbose flag, turn it on ASAP
            if( argS == "-V" ) verbose = true;
            else
            {
                // Version request
                if( argS == "-v" ) cmdLine["v"] = "1";

                else 
                {
                    // USB Camera discovery
                    if( argS == "-l" ) cmdLine["l"] = "1";

                    else 
                    {
                        // Device query
                        if( argS == "-a" ) cmdLine["a"] = "1";

                        else 
                        {
                            // Query Video modes
                            if( argS == "-m" )
                            {
                                // check for another parameter, must be a number, between 0 and 63
                                if( (i < argc) && (is_number(argv[i])) ) cmdLine["m"] = argv[i++];

                                else 
                                {
                                    // print error message and bail
                                    outln( "ERR : invalid attribute for [-m]" );
                                    printBasicHelp();
                                    return -1;
                                }

                            } else 
                            {
                                // if we made it here then the command is invalid
                                outln( "ERR : invalid command [" + argS + "]" );
                                printBasicHelp();
                                return -1;
                            }
                        }
                    }
                }
            }
        }

        // now process the commands, some depend on others, some can be done in sequence 
        // ex. -l -a can both be completed in single cmd line request
        //

        // always print the version info if asked
        if( cmdLine["v"] == "1" ) printVersionInfo();

        // Video Mode - should be done alone, skip all other commands
        if( cmdLine["m"].length() > 0 )
        {
            listVideoModes( cmdLine["m"] ) ;
            return 1;
        }

        if( cmdLine["l"] == "1" ) listUSBCameras();

        if( cmdLine["a"] == "1" ) listAllDevices();
    }

    return 0;
}

// Functional demonstration functions
void listUSBCameras()
{
    std::map<int, V4l2Camera *> camList;

    outerr( "" );
    outerr( "USB camera discovery starting..." );

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
                            } else outerr( nam + " : zero video modes detected" );
                        } else outerr( nam + " : does not support video capture" );
                    } else outerr( nam + " : unable to query capabilities" );
                } else outerr( nam + " : failed to open device" );
            } else outerr( nam + " : device indicates unable to open" );
        } else outln( nam + " : failed to create V4l2Camera object for device" );

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
        } else outln( nam + " : error !!! - failed to create V4l2Camera object for device" );

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
                outln( vm.format_str + " : " + std::to_string(vm.width) + " x " + std::to_string(vm.height) );
            }
            outln( "" );
            outln( "..." + std::to_string(tmp->getVideoModes().size()) + " video modes supported" );
        }
    }
    delete tmp;

    outln( "" );
    outln( "...discovery complete" );

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

void printBasicHelp()
{
    outln( "" );
    outln( "Usage" );
    outln( "-----" );

    outln( "-h :            this message" );
    outln( "-v :            show version of the V4l2Camera sub system");
    outln( "-V :            operate in verbose mode (lots of information messages)");
    outln( "-l :            list all USB cameras in the /dev/videoX driver space" );
    outln( "-a :            identify all openable devices in /dev/videoX driver space" );
    outln( "-m [0..63] :    list all the video modes supported by camera /dev/video<number>" );
    outln( "" );
}

void outln( std::string line )
{
    std::cout << "[\x1b[32mrslt\x1b[0m] " << line << std::endl;
}

void outerr( std::string line )
{
    if( verbose ) std::cout << "[\x1b[1;33minfo\x1b[0m] " << line << std::endl;
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}