#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <map>
#include <string>
#include <cstring>

#include "wrapH264.h"

// Version Info
//
const int s_majorVersion = 0;
const int s_minorVersion = 1;
const int s_revision = 10;

int main( int argc, char** argv )
{
    // set the flags
    bool hasH264Header = false;
    bool inFileProvided = false;
    bool outFileProvided = false;
    bool printHelp = false;
    bool printVersion = false;

    std::string inFilename = "";
    std::string outFilename = "";

    std::ifstream inFile;
    std::fstream outFile;

    // parse the command line
    for( int i = 1; i < argc; i++ )
    {
        std::string cmp = argv[i];

        if( (cmp == "-h") || (cmp == "--help") ) printHelp = true;
        if( cmp == "-v" ) printVersion = true;

        if( cmp == "-i" ) 
        {
            // make sure there is a filename
            if( ((i+1) < argc) && (argv[i+1][0] != '-') )
            {
                inFilename = argv[++i];
                inFileProvided = true;
            } else 
            {
                std::cerr << "[\033[0;31mwarning\033[0;31] : invalid filename or filename not provided, exiting" << std::endl;
                return 1;
            }
        }

        if( cmp == "-o" ) 
        {
            // make sure there is a filename
            if( ((i+1) < argc) && (argv[i+1][0] != '-') )
            {
                outFilename = argv[++i];
                outFileProvided = true;
            } else 
            {
                std::cerr << "[\033[0;31mwarning\033[0;31] : invalid filename or filename not provided, exiting" << std::endl;
                return 1;
            }
        }
    }
        
    // check for command line arguments and then bail
    if( printHelp )
    {
        std::cout <<    "Usage: " << argv[0] << " " << getVersionString() << std::endl << std::endl <<
                        " [-h|--help]       : this message" << std::endl <<
                        " [-v]              : print version information" << std::endl <<
                        " [-i filename]     : H264 stream to process" << std::endl <<
                        "                     ...default is to decode from std:cin" << std::endl <<
                        " [-o filename]     : MP4 file to create" << std::endl;
        return 0;
    }

    // No help requested, then open the MP4 file
    //
    // do the version thing
    //
    if( printVersion ) { std::cout << argv[0] << " Version : " << getVersionString() << ", (c) copyright 2025, All rights reserved, slapfrog Labs" << std::endl; return 0; }

    // check fi both input and output files are provided
    if( !inFileProvided )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] Input filename not provided, exiting" << std::endl;
        return 1;
    }

    if( !outFileProvided )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] Output filename not provided, using output.mp4" << std::endl;
        outFilename = "output.mp4";
        outFileProvided = true;
    }

    // try to open the output file
    //
    outFile.open( outFilename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    if( !outFile.is_open() )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] could not open file for writing : " << outFilename << ", exiting" << std::endl;
        return 1;
    }

    // try to open the input file
    //
    inFile.open( inFilename, std::ios::in | std::ios::binary );
    if( !inFile.is_open() )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] could not open file for reading : " << inFilename << ", exiting" << std::endl;
        return 1;
    }

    // do the work
    std::cout << "[\033[1;34minfo\033[0m] Decoding MP4 stream from : " << inFilename << std::endl;
    std::cout << "[\033[1;34minfo\033[0m] Writing MP4 file to      : " << outFilename << std::endl;
    std::cout << std::endl;

    // run a pass on the input file, to determine the frame count and sizes
    //
    // get video parameters (width, height, frame rate etc)
    //
    struct mp4StreamInfo_t * streamInfo = getStreamInfo( inFile );
    if( streamInfo == NULL )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] could not parse MP4 stream" << std::endl;
        return 1;
    }

    // add more info to the stream header
    streamInfo->timeScale = 1000;
    streamInfo->timeScale_swapped = swapEndian(streamInfo->timeScale);
    streamInfo->ticks_per_frame = (unsigned int)(streamInfo->timeScale / streamInfo->frame_rate);

    float dur_secs = (float)streamInfo->frame_count / (float)streamInfo->frame_rate;
    unsigned int dur_ticks = (unsigned int)(dur_secs * (float)streamInfo->timeScale);

    std::cout << "[\033[1;34minfo\033[0m] Stream Info : " << "width: " << streamInfo->width << ", height: " << streamInfo->height << ", frame Rate: " << streamInfo->frame_rate << std::endl;
    std::cout << "[\033[1;34minfo\033[0m] duration: " << dur_secs << " secs, " << dur_ticks << ", frame count = " << streamInfo->frame_count << std::endl;
    std::cout << "[\033[1;34minfo\033[0m] ticks: " << dur_ticks << " at " << streamInfo->timeScale << " ticks per sec" << std::endl << std::endl;

    streamInfo->videoDuration = dur_ticks;

    // rewind the input stream
    inFile.clear();
    inFile.seekg( 0, std::ios::beg );

    //
    // Write the minimal MP4 header to contain the H264 stream
    //
    if( !writeMP4Header( outFile, streamInfo ) )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] could not write MP4 header, exiting" << std::endl;
        return 1;
    }

    int mdatStartLocation = outFile.tellp();

    if( !writeMDATatom( inFile, outFile ) )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] could not write MDAT (raw data) atom, exiting" << std::endl;
        return 1;
    }

    // run a final verification on the file content
    //
    if( !verifyMP4Data( outFile, streamInfo, mdatStartLocation ) )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] could not verify MP4 header, exiting" << std::endl;
        return 1;
    }

    // close the file
    //
    inFile.close();
    outFile.close();

    // indicate done
    std::cerr << std::endl << "[\033[1;34minfo\033[0m] " << argv[0] << "....done" << std::endl;

    return 0;
}
