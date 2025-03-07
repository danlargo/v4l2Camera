#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <map>
#include <string>

#include "walkMP4.h"

// Version Info
//
const int s_majorVersion = 0;
const int s_minorVersion = 1;
const int s_revision = 10;

// Formatting depth counter
unsigned int m_depth;

int main( int argc, char** argv )
{
    // set the flags
    bool fileNameProvided = false;
    bool printHelp = false;
    bool printVersion = false;
    bool showDictionary = false;

    std::string filename = "";
    char * buffer;
    std::ifstream file;
    struct atom_t atom;

    // parse the command line
    for( int i = 1; i < argc; i++ )
    {
        std::string cmp = argv[i];

        if( (cmp == "-h") || (cmp == "--help") ) printHelp = true;
        if( cmp == "-v" ) printVersion = true;
        if( cmp == "-s" ) showDictionary= true;
        if( cmp == "-i" ) 
        {
            // make sure there is a filename
            if( ((i+1) < argc) && (argv[i+1][0] != '-') )
            {
                filename = argv[++i];
                fileNameProvided = true;
            } else std::cerr << "[\033[0;31mwarning\033[0;31] : invalid filename or filename not provided, defaulting to STDIN" << std::endl;
        }
    }
        
    // check for command line arguments and then bail
    if( printHelp )
    {
        std::cout <<    "Usage: " << argv[0] << " " << getVersionString() << std::endl << std::endl <<
                        " [-h|--help]       : this message" << std::endl <<
                        " [-v]              : print version information" << std::endl <<
                        " [-s]              : show the MP4 data dictionary used for parsing rules" << std::endl <<
                        " [-i filename]     : MP4 file to process" << std::endl <<
                        "                     ...default is to decode from std:cin" << std::endl; 
        return 0;
    }

    // Parse the dictionary
    //
    std::cout << std::endl;
    std::map<std::string, struct node_t*> dictionary = parseDictionary( "./walkMP4_dictionary.json" );

    if( dictionary.size() == 0 )
    {
        std::cerr << "[\033[1;31mfatal\x1b[0m] : could not parse dictionary or file not found" << std::endl;
        return 1;
    }

    // check if we are just printing the dictionary
    //
    if( showDictionary ) { printDictionary( dictionary ); return 0; }

    // No help requested, then open the MP4 file or STDIN
    //
    // do the version thing
    //
    if( printVersion ) { std::cout << argv[0] << " Version : " << getVersionString() << ", (c) copyright 2025, All rights reserved, slapfrog Labs" << std::endl; return 0; }

    // try to open the file
    if( !fileNameProvided ) filename = "/dev/stdin";

    file.open( filename, std::ios::in | std::ios::binary );
    if( !file.is_open() )
    {
        std::cerr << "[\033[0;31mwarning\033[0m] : could not open file " << filename << std::endl;
        return 1;
    }

    // do the work
    std::cout << "[\033[1;34minfo\033[0m] : Decoding MP4 stream ";

    if( fileNameProvided ) std::cout << "from file " << filename << std::endl;
    else std::cout << "from STDIN" << std::endl;

    std::cout << std::endl;

    // read the file
    // MP4 file format defined here https://www.loc.gov/preservation/digital/formats/fdd/fdd000155.shtml
    //
    // series of ATOM tags followed by data
    //
    // we know the file is a list of atoms, so jump right in
    //
    parseATOM( file, dictionary, -1 );

    // close the file
    //
    file.close();

    // indicate done
    std::cerr << std::endl << std::endl << argv[0] << " : done" << std::endl;

    return 0;
}
