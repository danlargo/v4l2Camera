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


void captureFrame( std::string deviceID, std::string fileName, std::string format, std::string addHeader )
{
    bool sendToStdout = true;

    std::ofstream outFile;

    // validate imag format
    if (format.length() > 0)
    {
		if (format == "jpg" || format == "bmp" || format == "raw") {}
		else 
        {
            format = "raw";
            outwarn("Invalid or no image format specified, defaulting to <raw>");
		}
    } else {
        format = "raw";
        outwarn("No format specified, writing <raw> output data");
    }

    // check if filename is specified
    if( fileName.length() > 0 ) sendToStdout = false;
    else outwarn( "No filename specified, writing <raw> output data (image frame) to STDOUT");

    // open the output file
    if( !sendToStdout ) 
    {
        outFile.open( fileName, std::ios::trunc | std::ios::binary );
        if( !outFile.is_open() ) 
        {
#ifdef _WIN32
            std::array<char, 256> errorBuffer;
            strerror_s(errorBuffer.data(), errorBuffer.size(), errno);
            outerr("Failed to open output file : " + fileName + " - " + std::string(errorBuffer.data()));
#else
            outerr( "Failed to open output file : " + fileName + " - " + strerror(errno) );
#endif
            return;
        }
    }

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
    outinfo( "Using " + cam->getDevName() + " : " + cam->getUserName() + " for image capture");

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
            // grab a single frame
            struct v4l2cam_image_buffer* inB = cam->fetch(true);
            if( inB && inB->buffer )
            {
                // check for invalid JPG file (if MJPG imag format)
                if( "MJPG" == data->format_str )
                {
                    // check the header, should be 0xFF 0xD8 0xFF 
                    if( (inB->buffer[0] == 0xff) && (inB->buffer[1] == 0xd8) && ((int)inB->buffer[2] == 0xff) ) 
                    {} else
                    {
                        outwarn( "Invalid JPG header, should be [FFD8FF], it is actually [" 
                                    + makeHexString(inB->buffer, 3, true )
                                    + "] : buflen is " 
                                    + std::to_string(inB->length) );
                        outwarn( "...repeating fetch" );

                        // retry the fetch (10 times at most)
                        int tries = 0;
                        bool goodFrame = false;
                        while( tries < 10 )
                        {
                            // re have to re-initialize as we didn't queue up a buffer last time
                            cam->close();
                            if( cam->open() && cam->init( v4l2cam_fetch_mode::userPtrMode ) )
                            {
                                // free the previous buffers
                                delete inB->buffer;
                                delete inB;

                                struct v4l2cam_image_buffer * inB = cam->fetch(true);
                                if( inB && inB->buffer )
                                {
                                    if( (inB->buffer[0] == 0xff) && (inB->buffer[1] == 0xd8) && ((int)inB->buffer[2] == 0xff) ) 
                                    {
                                        goodFrame = true;
                                        break;
                                    }
                                    // do it again
                                    outwarn( "...and again" );
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
                        if( goodFrame ) outinfo( "...got good frame" );
                    }
                }

                if (inB)
                {
                    // make sure MJPG gets output as <jpg>
                    //
                    if ("MJPG" == data->format_str)
                    {
                        if (format != "jpg")  outwarn("Invalid or no format specified for Motion-JPEG capture, defaulting to <jpg> format");

                        // output as native JPEG (basically raw output)
                        if (sendToStdout) std::cout.write((char*)inB->buffer, inB->length);
                        else outFile.write((char*)inB->buffer, inB->length);

                    } else if(format == "bmp" )
                    {
                        // try to convert to RGB24 format
                        unsigned char* rgbBuffer = nullptr;
                        if (data->fourcc == cam->fourcc_charArray_to_int((unsigned char*)"YUYV")) rgbBuffer = yuv422ToRGB(inB->buffer, data->width, data->height, false);
                        else if (data->fourcc == cam->fourcc_charArray_to_int((unsigned char*)"YVYU")) rgbBuffer = yvu422ToRGB(inB->buffer, data->width, data->height, false);
                        else if (data->fourcc == cam->fourcc_charArray_to_int((unsigned char*)"YUY2")) rgbBuffer = yuy2422ToRGB(inB->buffer, data->width, data->height, false);
                        else if (data->fourcc == cam->fourcc_charArray_to_int((unsigned char*)"YU12")) rgbBuffer = planarYUV420ToRGB(inB->buffer, data->width, data->height, false);
                        else if (data->fourcc == cam->fourcc_charArray_to_int((unsigned char*)"I420")) rgbBuffer = planarYVU420ToRGB(inB->buffer, data->width, data->height, false);
                        else if (data->fourcc == cam->fourcc_charArray_to_int((unsigned char*)"NV12")) rgbBuffer = interleavedYUV420ToRGB(inB->buffer, data->width, data->height, false);
                        else if (data->fourcc == cam->fourcc_charArray_to_int((unsigned char*)"NV21")) rgbBuffer = interleavedYVU420ToRGB(inB->buffer, data->width, data->height, false);
                        else if (data->fourcc == cam->fourcc_charArray_to_int((unsigned char*)"Y16 ")) rgbBuffer = gs16ToRGB(inB->buffer, data->width, data->height, false);
                        else if (data->fourcc == cam->fourcc_charArray_to_int((unsigned char*)"Y8  ")) rgbBuffer = gs8ToRGB(inB->buffer, data->width, data->height, false);
                        else if (data->fourcc == cam->fourcc_charArray_to_int((unsigned char*)"Y800")) rgbBuffer = gs8ToRGB(inB->buffer, data->width, data->height, false);
                        else if (data->fourcc == cam->fourcc_charArray_to_int((unsigned char*)"GREY")) rgbBuffer = gs8ToRGB(inB->buffer, data->width, data->height, false);

                        if( rgbBuffer )
                        {
                            // close the current file attempt
                            outFile.close();
                            // output as BMP image, if fileName is blank then will send to stdout
                            saveRGB24AsBMP(rgbBuffer, data->width, data->height, fileName );
                            delete rgbBuffer;
                        }
                        else {
                            // unable to convert, output as raw
                            outwarn("Unable to convert to RGB format, outputting raw image data");
                            // output as raw image data
                            if (sendToStdout) std::cout.write((char*)inB->buffer, inB->length);
                            else outFile.write((char*)inB->buffer, inB->length);
                        }
                    }  else {
                        // output as raw image data
                        if (sendToStdout) std::cout.write((char*)inB->buffer, inB->length);
                        else outFile.write((char*)inB->buffer, inB->length);
                    }

                    // delete the returned data
                    if( inB->buffer ) delete inB->buffer;
                    delete inB;
                }

            } else outwarn( "Nothing returned from fetch call for : " + cam->getDevName() + " " + cam->getUserName() );
        } else outerr( "Failed to initilize fetch mode for : " + cam->getDevName() + " " + cam->getUserName() );

        // close the camera
        cam->close();

    } else outerr( "Failed to create/open camera : + deviceID" );

    // delete the camera object
    for( const auto &x : camList ) delete x;

    // close the file
    if( !sendToStdout ) outFile.close();
    else std::cout.flush();

}

void captureFrames( std::string deviceID, std::string timeDuration, std::string fileName, std::string addHeader )
{
    bool sendToStdout = true;
    std::ofstream outFile;
    
    int timeToCapture = 10;
    int fpsVideo = 30;      // default is 30 fps
    int framesToCapture;
    int actualFps = fpsVideo;
    int actualFrameCount = 0;

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

    // check if filename is specified
    if( fileName.length() > 0 ) 
    {
        outinfo( "Using " + cam->getDevName() + " : " + cam->getUserName() +  " for video capture" );
        sendToStdout = false;
    }
    else outinfo( "No filename specified, writing output data (video frames) to STDOUT");

    // check the time duration
    if( timeDuration.length() > 0 ) timeToCapture = std::stoi( timeDuration );
    else outinfo( "No time duration specified, defaulting to 10 secs");

    // check for max duration
    if( timeToCapture > 120 )
    {
        outwarn( "Max capture time is 120 secs, setting to 120 secs" );
        timeToCapture = 120;
    }
    
    // initiate video (multiple frame) capture
    framesToCapture = timeToCapture * fpsVideo;

    // open the output file
    if( !sendToStdout ) 
    {
        outFile.open( fileName, std::ios::trunc | std::ios::binary );
        if( !outFile.is_open() ) 
        {
#ifdef _WIN32
            std::array<char, 256> errorBuffer;
            strerror_s(errorBuffer.data(), errorBuffer.size(), errno);
            outerr("Failed to open output file : " + fileName + " - " + std::string(errorBuffer.data()));
#else
            outerr( "Failed to open output file : " + fileName + " - " + strerror(errno) );
#endif
            return;
        }
    }

    if( verbose ) cam->setLogMode( v4l2cam_logging_mode::logToStdOut );
    else cam->setLogMode( v4l2cam_logging_mode::logOff );

    if( cam && cam->open() )
    {
        // display the current video mode and frame rate
        struct v4l2cam_video_mode * data = cam->getFrameFormat();
        int data2 = fpsVideo;
        if( data )
        {
            // grab the frame format
            data2 = cam->getFrameRate();
            //  update frame rate and frames to capture based on the retrieved frame rate
            fpsVideo = data2;
            framesToCapture = timeToCapture * fpsVideo;
            // tell the user what we are doing
            outinfo( "   ...using format : " + data->format_str + 
                    " @ " + std::to_string(data->width) + " x " + std::to_string(data->height) + 
                    " : " + std::to_string(data2) + " fps" );

        } else outwarn( "Failed to fetch current video format for : " + cam->getDevName() + " " + cam->getUserName() + ", using default of 30 fps" );

        outinfo( "   ...video capture duration is : " + std::to_string(timeToCapture) + " seconds, " + std::to_string(framesToCapture) + " frames" );

        // initialize the camera
        if( cam->init( v4l2cam_fetch_mode::userPtrMode ) )
        {
            // set up the fpsVideo timers
            std::chrono::steady_clock::time_point start;
            typedef std::chrono::duration<int,std::milli> millisec_t ;
            millisec_t delta;

            start = std::chrono::steady_clock::now();

            // grab a a bunch of frames
            if( (addHeader.length() > 0) && (data->format_str == "H264") ) outinfo( "Adding H264 header to frames" );
            // start the calc fps at the requeted fps
            actualFps = fpsVideo;

            while( framesToCapture > 0 )
            {
                bool goodFrame = false;
                
                // fetch will block if no frame is available 
                struct v4l2cam_image_buffer * inB = cam->fetch(framesToCapture == 1);

                // check the return buffers
                if( inB && inB->buffer )
                {

                    // check for invalid JPG file (if Motion-JPEG selected)
                    if( "MJPG" == data->format_str )
                    {
                        // check the header, should be 0xFF 0xD8 0xFF 
                        if( (inB->buffer[0] == 0xff) && (inB->buffer[1] == 0xd8) && ((int)inB->buffer[2] == 0xff) ) goodFrame = true;

                    } else goodFrame = true;

                    if( goodFrame )
                    {
                        actualFrameCount++;

                        // calculate how much time we are actually waiting
                        delta = std::chrono::duration_cast<millisec_t> (std::chrono::steady_clock::now() - start );
                        if( delta.count() > 0 ) actualFps = (1000*actualFrameCount) / delta.count();
                        else actualFps = fpsVideo;

                        // add header if requested and this is an H264 frame
                        if( (addHeader.length() > 0) && (data->format_str == "H264") )
                        {
                            // send the actual FPS to the other side, just in case it skews during capture
                            char * h264Buffer = addH264Header( inB->buffer, inB->length, actualFps, data->width, data->height ); ;
                            if( h264Buffer )
                            {
                                if (sendToStdout) std::cout.write( h264Buffer, inB->length + sizeof( h264FrameHeader_t) );
                                else outFile.write( h264Buffer, inB->length + sizeof( h264FrameHeader_t) );
                                delete h264Buffer;
                            }
                        } else {
                            // write the buffer out to the file
                            if( sendToStdout ) std::cout.write( (char*)inB->buffer, inB->length );
                            else outFile.write( (char *)inB->buffer, inB->length );
                        }
                    } else outwarn( "Invalid frame returned, skipping" );

                    // delete the returned data
                    delete inB->buffer;
                    delete inB;

                } else outwarn( "Nothing returned from fetch call for : /dev/video" + deviceID );

                framesToCapture--;

            }

            // print out a summary message
            outinfo( "   ...actual capture rate was : " + std::to_string(actualFps) + " fps" );
            outinfo( "   ...actual frames captured : " + std::to_string(actualFrameCount) );

        } else outerr( "Failed to initilize fetch mode for : " + cam->getDevName() + " " + cam->getUserName()  );

        // close the camera
        cam->close();

    } else outerr( "Failed to create/open camera " + deviceID );

    // delete the camera object
    for( const auto &x : camList ) delete x;

    // close the file
    if( sendToStdout ) std::cout.flush();
    else outFile.close();

}

char * addH264Header( unsigned char * buffer, int length, int rate, int width, int height )
{

    struct h264FrameHeader_t frameHeader;

    // H264 header
    frameHeader.delimiter[0] = 's';
    frameHeader.delimiter[1] = 'l';
    frameHeader.delimiter[2] = 'a';
    frameHeader.delimiter[3] = 'p';

    frameHeader.frame_size = length;
    frameHeader.rate = rate;
    frameHeader.width = width;
    frameHeader.height = height;

    frameHeader.h264_frame_type = H264_UNKNOWN;
    frameHeader.sps_offset = 0;
    frameHeader.pps_offset = 0;
    frameHeader.frame_offset = 0;

    // walk the buffer to determine the type and offsets
    int i = 0;
    while( i < length )
    {
        // find the start of the next frame
        if( (buffer[i] == 0x00) && (buffer[i+1] == 0x00) && (buffer[i+2] == 0x00) && (buffer[i+3] == 0x01) )
        {
            // decode the format byte
            uint8_t forbidden = (buffer[i+4] >> 7) & 0x01;
            uint8_t nri = (buffer[i+4] >> 5) & 0x03;
            uint8_t type = buffer[i+4] & 0x1F;

            switch( type )
            {
                case 0x7: // SPS
                    frameHeader.sps_offset = i;
                    break;
                case 0x8: // PPS
                    frameHeader.pps_offset = i;
                    break;
                case 0x5: // IDR
                    frameHeader.h264_frame_type = H264_FRAME_I;
                    frameHeader.frame_offset = i;
                    break;
                case 0x1: // NDR
                    frameHeader.h264_frame_type = H264_FRAME_P;
                    frameHeader.frame_offset = i;
                    break;
                default:
                    frameHeader.h264_frame_type = type;
                    frameHeader.frame_offset = i;
                    break;
            }
        }
        i++;
    }

    // allocate a new buffer
    char * newBuffer = new char[ length + sizeof( h264FrameHeader_t ) ];
    if( newBuffer )
    {
        // copy the header into the new buffer
        unsigned int tmp;
        memcpy( newBuffer, (char*)&frameHeader.delimiter, 4 );
        tmp = swapEndian( frameHeader.rate );
        memcpy( newBuffer + 4, (char*)&tmp, 4 );
        tmp = swapEndian( frameHeader.width );
        memcpy( newBuffer + 8, (char*)&tmp, 4 );
        tmp = swapEndian( frameHeader.height );
        memcpy( newBuffer + 12, (char*)&tmp, 4 );

        tmp = swapEndian( frameHeader.h264_frame_type );
        memcpy( newBuffer + 16, (char*)&tmp, 4 );
        tmp = swapEndian( frameHeader.sps_offset );
        memcpy( newBuffer + 20, (char*)&tmp, 4 );
        tmp = swapEndian( frameHeader.pps_offset );
        memcpy( newBuffer + 24, (char*)&tmp, 4 );
        tmp = swapEndian( frameHeader.frame_offset );
        memcpy( newBuffer + 28, (char*)&tmp, 4 );
        tmp = swapEndian( frameHeader.frame_size );
        memcpy( newBuffer + 32, (char*)&tmp, 4 );

        // copy the data
        memcpy( newBuffer + 36, buffer, length );
    }

    return newBuffer;
}

// Endian Functions
//
unsigned int swapEndian( unsigned int in )
{
    return (in >> 24) | ((in << 8) & 0x00FF0000) | ((in >> 8) & 0x0000FF00) | (in << 24);
}

int swapEndian( int in )
{
    return (in >> 24) | ((in << 8) & 0x00FF0000) | ((in >> 8) & 0x0000FF00) | (in << 24);
}

unsigned short swapEndian( unsigned short in )
{
    return (in >> 8) | (in << 8);
}

unsigned long swapEndian( unsigned long in )
{
    return ((in >> 56) & 0x00000000000000FF) |
           ((in >> 40) & 0x000000000000FF00) |
           ((in >> 24) & 0x0000000000FF0000) |
           ((in >> 8)  & 0x00000000FF000000) |
           ((in << 8)  & 0x000000FF00000000) |
           ((in << 24) & 0x0000FF0000000000) |
           ((in << 40) & 0x00FF000000000000) |
           ((in << 56) & 0xFF00000000000000);
}