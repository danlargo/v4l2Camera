#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

#include "decodeH264.h"

// Version Info
//
const int s_majorVersion = 0;
const int s_minorVersion = 1;
const int s_revision = 10;

unsigned int m_timescale;
int m_depth;

int main( int argc, char** argv )
{
    // set the flags
    bool onlyFrameHeaders = false;
    bool fileNameProvided = false;
    bool printHelp = false;
    bool printVersion = false;
    bool printGlossary = false;

    std::string filename = "";
    char * buffer;
    std::ifstream file;
    struct atom_t atom;

    // parse the command line
    for( int i = 1; i < argc; i++ )
    {
        std::string cmp = argv[i];

        if( cmp == "-f" ) onlyFrameHeaders = true;
        if( (cmp == "-h") || (cmp == "--help") ) printHelp = true;
        if( cmp == "-v" ) printVersion = true;
        if( cmp == "-g" ) printGlossary = true;
        if( cmp == "-i" ) 
        {
            // make sure there is a filename
            if( ((i+1) < argc) && (argv[i+1][0] != '-') )
            {
                filename = argv[++i];
                fileNameProvided = true;
            } else std::cerr << "[\x1b[1;31mwarning\x1b[0m] : invalid filename or filename not provided, defaulting to STDIN" << std::endl;
        }
    }
        
    // check for command line arguments and then bail
    if( printHelp )
    {
        std::cout <<    "Usage: " << argv[0] << " " << getVersionString() << std::endl << std::endl <<
                        " [-h|--help]       : this message" << std::endl <<
                        " [-v]              : print version information" << std::endl <<
                        " [-g]              : print a glossary of atom symbol descriptions" << std::endl <<
                        " [-f]              : decode frame headers only, streaming from UVC camera" << std::endl <<
                        "                       ...default is to try to decode the MP4 stream header" << std::endl <<
                        " [-i filename]     : decode MP4 file" << std::endl <<
                        "                       ...default is to decode from std:cin" << std::endl; 
        return 0;
    }

    // do the version thing
    if( printVersion ) { std::cout << argv[0] << " Version : " << getVersionString() << ", (c) copyright 2025, All rights reserved, slapfrog Labs" << std::endl; return 0; }

    // try to open the file
    if( !fileNameProvided ) filename = "/dev/stdin";

    file.open( filename, std::ios::in | std::ios::binary );
    if( !file.is_open() )
    {
        std::cerr << "[\x1b[1;31mwarning\x1b[0m] : could not open file " << filename << std::endl;
        return 1;
    }

    // do the work
    if( onlyFrameHeaders ) std::cout << "Decoding MP4 frame headers ";
    else std::cout << "Decoding MP4 stream ";
    if( fileNameProvided ) std::cout << "from file " << filename << std::endl;
    else std::cout << "from STDIN" << std::endl;
    std::cout << std::endl;

    // read the file
    // MP4 file format defined here https://www.loc.gov/preservation/digital/formats/fdd/fdd000155.shtml
    //
    // series of ATOM tags followed by data
    //
    int num_atoms = 0;

    while( !file.eof() )
    {
        if( onlyFrameHeaders )
        {
            // DECODE the UVC camera frames
            //
            int frame_type = -1;
            char * frame = h264GrabFrame( file );

            // bail for now
            if( frame == NULL ) std::cerr << "[\x1b[1;31mwarning\x1b[0m] : could not grab frame" << std::endl;
            delete [] frame;
            
            break;        

        } else {
            // DECODE the entire MP$ file or stream
            //
            // read the atom and then decide that to do
            m_depth = 0;
            atom = getATOMhdr( file );

            // check if the tag reading triggered eof
            if( file.eof() ) break;

            // print out the atom tag
            num_atoms++;
            std::cout << std::setw(2) << num_atoms << " [\033[1;32m" << atom.tag << "\033[0m] ";

            // decode the FTYP atom
            //
            if( atom.tag == "FTYP" ) 
            {
                if( !parseFTYPatom( file, atom.size ) ) break;

            } 
            else if( atom.tag == "MDAT" ) parseMDATatom( file, atom.size );
            else if( atom.tag == "MOOV" ) parseMOOVatom( file, atom.size );
            else if( atom.tag == "FREE" ) parseFREEatom( file, atom.size );
            
            else parseUNKNatom( file, atom.size );
        }
    }

    // close the file
    file.close();

    // print the glossary if asked
    if( printGlossary )
    {
        std::cout << std::endl << "Glossary of Atom Symbols" << std::endl;
        std::cout << "------------------------" << std::endl;
        std::cout << "ftyp : File Type" << std::endl;
        std::cout << "moov : Movie" << std::endl;
        std::cout << "mvhd : Movie Header" << std::endl;
        std::cout << "iods : Initial Object Descriptor" << std::endl;
        std::cout << "trak : Track" << std::endl;
        std::cout << "tkhd : Track Header" << std::endl;
        std::cout << "mdia : Media" << std::endl;
        std::cout << "mdhd : Media Header" << std::endl;
        std::cout << "hdlr : Handler" << std::endl;
        std::cout << "minf : Media Information" << std::endl;
        std::cout << "vmhd : Video Media Header" << std::endl;
        std::cout << "dinf : Data Information" << std::endl;
        std::cout << "dref : Data Reference" << std::endl;
        std::cout << "stbl : Sample Table" << std::endl;
        std::cout << "stsd : Sample Description" << std::endl;
        std::cout << "stts : Time-to-Sample" << std::endl;
        std::cout << "stsc : Sample-to-Chunk" << std::endl;
        std::cout << "stsz : Sample Size" << std::endl;
        std::cout << "stco : Chunk Offset" << std::endl;
        std::cout << "udta : User Data" << std::endl;
        std::cout << "free : Free Space" << std::endl;
        std::cout << "mdat : Media Data" << std::endl;
        std::cout << "cmov : Compressed Movie" << std::endl;
    }

    // indicate done
    std::cerr << std::endl << std::endl << argv[0] << " : done" << std::endl;

    return 0;
}
