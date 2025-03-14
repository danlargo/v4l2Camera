#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <map>
#include <string>

#include "wrapMP4.h"

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

    char * buffer;
    std::ifstream inFile;
    std::ofstream outFile;

    // counters
    int frame_count = 0;
    int frame_rate = -1;

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
            } else std::cerr << "[\033[0;31mwarning\033[0;31] : invalid filename or filename not provided, defaulting to STDIN" << std::endl;
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
                std::cerr << "[\033[0;31mwarning\033[0;31] : invalid filename or filename not provided" << std::endl;
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

    // No help requested, then open the MP4 file or STDIN
    //
    // do the version thing
    //
    if( printVersion ) { std::cout << argv[0] << " Version : " << getVersionString() << ", (c) copyright 2025, All rights reserved, slapfrog Labs" << std::endl; return 0; }

    // try to open the output file
    //
    if( !outFileProvided )
    {
        std::cerr << "[\033[0;31mwarning\033[0;31] : Output filename not provided" << std::endl;
        return 1;
    }

    outFile.open( outFilename, std::ios::out | std::ios::binary | std::ios::trunc);
    if( !outFile.is_open() )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] : could not open file for writing" << outFilename << std::endl;
        return 1;
    }

    // try to open the input file
    //
    if( !inFileProvided ) inFilename = "/dev/stdin";

    inFile.open( inFilename, std::ios::in | std::ios::binary );
    if( !inFile.is_open() )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] : could not open file for reading " << inFilename << std::endl;
        return 1;
    }

    // do the work
    std::cout << "[\033[1;34minfo\033[0m] : Decoding MP4 stream ";

    if( inFileProvided ) std::cout << "from file " << inFilename << std::endl;
    else std::cout << "from STDIN" << std::endl;

    std::cout << "[\033[1;34minfo\033[0m] : Writing MP4 file " << outFilename << std::endl;

    std::cout << std::endl;


    // read the file
    // MP4 file format defined here https://www.loc.gov/preservation/digital/formats/fdd/fdd000155.shtml
    //

    // read the first frame header to get the width and height
    //
    int width = 0;
    int height = 0;
    
    struct h264FrameHeader_t * header = getFrameHeader( inFile );
    if( header != NULL )
    {
        width = header->width;
        height = header->height;
        frame_rate = header->rate;
        delete header;
    } else
    {
        std::cerr << "[\033[0;31mwarning\033[0m] : could not read frame header" << std::endl;
        return 1;
    }

    //
    // Write the minimal MP4 header to contain the H264 stream
    //
    if( !writeMP4Header( outFile, width, height ) )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] : could not write MP4 header" << std::endl;
        return 1;
    }

    // now we loop until EOF is detected
    while( !inFile.eof() )
    {
        unsigned char * buf = new unsigned char[header->frame_size];

        // read the frame
        inFile.read( (char*)buf, header->frame_size );

        // write the frame
        writeRawData( outFile, (char*)buf, header->frame_size );
        delete [] buf;

        frame_count++;

        // check if file is done before waiting on the next header
        if( inFile.eof() ) break;

        // read the next frame header
        header = getFrameHeader( inFile );
        if( header == NULL ) break;
    }

    // cleanup
    updateAllSizeAndDuration( outFile, frame_rate, frame_count );

    // close the file
    //
    inFile.close();
    outFile.close();

    // indicate done
    std::cerr << std::endl << argv[0] << " : done" << std::endl;

    return 0;
}
