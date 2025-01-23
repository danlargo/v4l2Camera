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
        v4l2cam_logging_mode t = v4l2cam_logging_mode::logOff;
        if( verbose ) t = v4l2cam_logging_mode::logToStdOut;
        camList = LinuxCamera::discoverCameras(t);
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
    #endif

    // output the collected information
    outln( "----------------------------------" );
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
                std::string out = "[" + std::to_string(camIndex++) + "] "
                        + x->getDevName() + " : " + x->getUserName() + ", ";
                if( x->getVideoModes().size() > 0 ) out += std::to_string(x->getVideoModes().size()) + " video modes, ";
                if( x->getControls().size() > 0 ) out += std::to_string(x->getControls().size()) + " user controls, ";
                        
                char tmp[5]; tmp[4] = 0;
                x->fourcc_int_to_charArray(x->getMetaMode(), tmp);
                if( x->getMetaMode() > 0 ) 
                {
                    out += tmp;
                    out += " meta format" ;
                }

                outln( out );
                // output the capabilities info
                if( 0 == x->m_capabilities ) outln( "\r\n    devcaps...none" );
                else 
                {
                    outln( "    devcaps..." );
                    for( const auto &y : x->capabilitiesToStr() ) { outln ( "     > " + y ); }
                    outln( "" );
                }
            } else outerr( "Failed to open device : " + x->getDevName() );
            // close the camera now that we are done with it
            x->close();
        } else outerr( "Invalid V4l2Camera pointer in list" );
    }
    // delete all the cameras
    for( const auto &x : camList ) delete x;

    outln( "" );
    outln( "...discovery complete" );
}

void listAllDevices()
{
    #ifdef __linux__
        outln( "----------------------------------" );
        outln( "Devices in range /dev/video[0..63]" );
        outln( "" );

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
        v4l2cam_logging_mode t = v4l2cam_logging_mode::logOff;
        if( verbose ) t = v4l2cam_logging_mode::logToStdOut;
        camList = LinuxCamera::discoverCameras(t);
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

    outln( "------------------------------");
    outln( "Video Mode(s) for camera [" + deviceID + "] " + tmp->getDevName() + " : " + tmp->getUserName() );
    outln( "" );

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

    outln( "" );
    outln( "...discovery complete" );

}

void listUserControls( std::string deviceID )
{
    // create the camera object
    #ifdef __linux__
        std::vector< LinuxCamera *> camList;
        v4l2cam_logging_mode t = v4l2cam_logging_mode::logOff;
        if( verbose ) t = v4l2cam_logging_mode::logToStdOut;
        camList = LinuxCamera::discoverCameras(t);
        LinuxCamera * tmp = camList[std::stoi(deviceID)];
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * tmp = camList[std::stoi(deviceID)];
    #endif

    outln( "------------------------------");
    outln( "User Control(s) for " + tmp->getDevName() + " : " + tmp->getUserName() );
    outln( "" );

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


void fetchMetaData( std::string deviceID )
{
    // create the camera object
    #ifdef __linux__
        std::vector< LinuxCamera *> camList;
        v4l2cam_logging_mode t = v4l2cam_logging_mode::logOff;
        if( verbose ) t = v4l2cam_logging_mode::logToStdOut;
        camList = LinuxCamera::discoverCameras(t);
        LinuxCamera * tmp = camList[std::stoi(deviceID)];
    #elif __APPLE__
        std::vector<MACCamera *> camList;
        camList = MACCamera::discoverCameras();
        MACCamera * tmp = camList[std::stoi(deviceID)];
    #endif

    outln( "" );
    outln( "------------------------------");
    std::string out = "MetaData for (";

    char code4[5]; code4[4] = 0;
    tmp->fourcc_int_to_charArray( tmp->getMetaMode(), code4 );
    out += code4;
    out += " format) " + tmp->getDevName() + " : " + tmp->getUserName();
    outln( out );

    if( verbose ) tmp->setLogMode( v4l2cam_logging_mode::logToStdOut );
    else tmp->setLogMode( v4l2cam_logging_mode::logOff );

    if( tmp )
    {
        if( tmp->open() )
        {
            struct v4l2cam_metadata_buffer * data = tmp->fetchMetaData();
            if( data )
            {
                outln( "   ...got " + std::to_string(data->length) + " bytes of meta data" );
                // display the data in V4L2_META_FMT_UVC format
                struct uvch_data
                {
                    uint64_t timestamp;
                    uint16_t framenum;
                    uint8_t len;
                    uint8_t flags;
                    uint8_t data[];
                };
                struct uvch_data * uvch = (struct uvch_data *)data->buffer;
                outln( "   ...timestamp : " + std::to_string(uvch->timestamp) );
                outln( "   ...frame number : " + std::to_string(uvch->framenum) );
                outln( "   ...length : " + std::to_string(uvch->len) );
                outln( "   ...flags : " + std::to_string(uvch->flags) );

            } else outerr( "Failed to fetch meta data for : " + tmp->getDevName() + " " + tmp->getUserName() );

            tmp->close();
        }
    }
    // delete all the camera object now that we are done with them
    for( const auto &x : camList ) delete x;

    outln( "");
    outln( "...discovery complete" );

}