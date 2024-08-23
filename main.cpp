#include <iostream>
#include <map>

#include "defines.h"

#include "v4l2camera.h"

int main( int argc, char** argv )
{
    std::cout   << "Welcome to " << argv[0] << "sample and test application" 
                << "   ...using v4l2Camera " << v4l2Camera::getVersionString() 
                << " " << v4l2Camera::getCodeName()
                << std::endl;

    // check for command line arguments
    if( argc == 1 ) printBasicHelp();
    else
    {
        // parse the command options
        int i = 1;
        while( i < argc )
        {
            std::string argS = argv[i++];

            // show the help message
            if( argS == "-h" )
            {
                printBasicHelp();
                continue;
            }

            // show the verion information
            if( argS == "-v" )
            {
                printVersionInfo();
                continue;
            }

            // find and list the USB cameras
            if( argS == "-l" ) 
            {
                listUSBCameras();
                continue;
            }

            // try to open all the /dev/videoX devices, list the ones that open successfully
            if( argS == "-a" ) 
            {
                listAllDevices();
                continue;
            }

            // display the error message
            invalidCommandMsg( argS );

        }
    }

    return 0;
}

// Functional demonstration functions
void listUSBCameras()
{
    std::map<int, v4l2Camera *> camList;

    outln( "" );
    outln( "progress..." );
    outln( "...walking device list to find USB cameras" );
    outln( "" );

    for( int i=0; i<64; i++ )
    {
        bool keep = false;

        // build the device name
        std::string nam = "/dev/video" + std::to_string(i);

        // create the camera object
        v4l2Camera * tmpC = new v4l2Camera(nam);

        if( tmpC )
        {
            if( tmpC->canOpen() )
            {
                // open the camera so we can query all its capabilities
                if( tmpC->open() )
                {
                    if( tmpC->getCaps() )
                    {
                        if( tmpC->canFetch() )
                        {
                            // have it query its own capabilities
                            tmpC->enumControls();
                            tmpC->enumVideoModes();

                            if( tmpC->myModes.size() > 0 )
                            {
                                // save the camera for display later
                                keep = true;
                                camList[i] = tmpC;
                            } else outln( nam + " : zero video modes detected" );
                        } else outln( nam + " : does not support video capture" );
                    } else outln( nam + " : unable to query capabilities" );
                } else outln( nam + " : failed to open device" );
            } else outln( nam + " : device indicates unable to open" );
        } else outln( nam + " : failed to create v4l2Camera object for device" );

        if( !keep ) delete tmpC;
    }

    // output the collected information
    outln( "" );
    outln( "...displaying results" );
    outln( "" );
    outln( "Detected " + std::to_string(camList.size()) + " USB Camera(s)" );
    outln( "" );

    for( auto x : camList )
    {
        v4l2Camera * tmp = x.second;

        outln( tmp->getFidName() + " : " + tmp->getDevName() + ", " 
                + std::to_string(tmp->myModes.size()) + " video modes, "
                + std::to_string(tmp->myControls.size()) + " user controls"
            );

        // delete the camera now that we are done with it
        delete tmp;
    }

    outln( "" );
    outln( "...walk is complete" );
}

void listAllDevices()
{
    outln( "" );
    outln( "The following devices can be opened" );
    outln( "-----------------------------------" );

    int numFound = 0;

    for( int i=0; i<64; i++ )
    {

        // build the device name
        std::string nam = "/dev/video" + std::to_string(i);

        // create the camera object
        v4l2Camera * tmpC = new v4l2Camera(nam);

        if( tmpC )
        {
            if( tmpC->canOpen() ) 
            {
                outln( nam + " : can be opened" );
                numFound++;
            }
        } else outln( nam + " : error !!! - failed to create v4l2Camera object for device" );

        delete tmpC;
    }

    outln( "" );
    outln( "...found : " + std::to_string(numFound) + " devices that can be opened by user" );
    outln( "" );
    outln( "...walk is complete" );
}

// Basic Helper Functions
void printVersionInfo()
{
    outln("");
    outln( "v4l2Camera - version info");
    outln( "-------------------------");
    outln( "Version     : " + v4l2Camera::getVersionString() );
    outln( "Code Name   : " + v4l2Camera::getCodeName() );
    outln( "Last Commit : " + v4l2Camera::getLastMsg() );
    outln( "" );
}

void printBasicHelp()
{
    outln( "" );
    outln( "Usage" );
    outln( "-----" );

    outln( "-h :        this message" );
    outln( "-v :        show version of the v4l2Camera sub system");
    outln( "-l :        list all USB cameras in the /dev/videoX driver space" );
    outln( "-a :        identify all openable devices in /dev/videoX driver space" );
    outln( "-m 0/63 :   list all the video modes supported by camera /dev/video<number>" );
}

void invalidCommandMsg( std::string invalidCmd )
{
    outln( "" );
    outln( "Error invalid command : "  + invalidCmd );
    outln( "" );
    printBasicHelp();
}

void outln( std::string line )
{
    std::cout << line << std::endl;
}
