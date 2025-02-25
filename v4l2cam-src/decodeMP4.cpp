#include <iostream>
#include <fstream>

const int s_majorVersion = 0;
const int s_minorVersion = 1;
const int s_revision = 10;

std::string getVersionString() 
{ 
    std::string ver = "v"; 
    ver += std::to_string(s_majorVersion); ver += "."; 
    ver += std::to_string(s_minorVersion); ver += "."; 
    ver += std::to_string(s_revision);  
    return ver;
}

unsigned int swapOrder( unsigned int in )
{
    return (in >> 24) | ((in << 8) & 0x00FF0000) | ((in >> 8) & 0x0000FF00) | (in << 24);
}

int main( int argc, char** argv )
{
    // set the flags
    bool onlyFrameHeaders = false;
    bool fileNameProvided = false;
    bool printHelp = false;
    bool printVersion = false;

    std::string filename = "";

    // parse the command line
    for( int i = 1; i < argc; i++ )
    {
        std::string cmp = argv[i];

        if( cmp == "-f" ) onlyFrameHeaders = true;
        if( (cmp == "-h") || (cmp == "--help") ) printHelp = true;
        if( cmp == "-v" ) printVersion = true;
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
    if( (argc == 1) || printHelp)
    {
        std::cout <<    "Usage: " << argv[0] << " " << getVersionString() << std::endl << std::endl <<
                        " [-h|--help]       : this message" << std::endl <<
                        " [-v]              : print version information" << std::endl <<
                        " [-f]              : decode frame headers only, streaming from UVC camera" << std::endl <<
                        "                       ...default is to try to decode the MP4 stream header" << std::endl <<
                        " [-i filename]     : decode MP4 file" << std::endl <<
                        "                       ...default is to decode from std:cin" << std::endl; 
        return 0;
    }

    // do the version thing
    if( printVersion )
    {
        std::cout << argv[0] << " Version : " << getVersionString() << ", (c) copyright 2025, All rights reserved, slapfrog Labs" << std::endl;
        return 0;
    }

    // try to open the file
    std::ifstream file;
    if( fileNameProvided )
    {
        file.open( filename, std::ios::in | std::ios::binary );
        if( !file.is_open() )
        {
            std::cerr << "[\x1b[1;31mwarning\x1b[0m] : could not open file " << filename << std::endl;
            return 1;
        }
    }

    // do the work
    if( onlyFrameHeaders ) std::cout << "Decoding MP4 frame headers ";
    else std::cout << "Decoding MP4 stream ";
    if( fileNameProvided ) std::cout << "from file " << filename << std::endl;
    else std::cout << "from STDIN" << std::endl;

    // read the file
    // MP$ file format defined here https://www.loc.gov/preservation/digital/formats/fdd/fdd000155.shtml
    //
    // series of ATOM tags followed by data
    //
    struct atom_t
    {
        unsigned int size;
        unsigned char type[4];
    };

    //
    // FTYP header format
    // Size             : A 32-bit unsigned integer representing the total size of the "ftyp" atom in bytes. 
    // Type             : A 4-byte code, always "ftyp", signifying the atom type. 
    // Major Brand      : A 4-byte code identifying the primary file format (usually "isom"). 
    // Minor Version    : A 32-bit unsigned integer representing the version number of the file format. 
    // Compatible Brands (Optional) : A list of additional 4-byte codes indicating compatible file formats.
    //
    struct ftyp_t
    {
        unsigned char majorBrand[4];
        unsigned int minorVersion;
    };

    char * buffer;

    bool done = false;

    while( !done )
    {
        // read the atom and then decide that to do
        struct atom_t atom;
        if( fileNameProvided ) 
        {
            file.read( (char *)&atom, 8 );
            if( file.eof() ) done = true;
        } else 
        {
            std::cin.read( (char *)&atom, 8 );
            if( std::cin.eof() ) done = true;
        }
        
        // re-build the size
        int real_size = swapOrder(atom.size);
        // print out the atom tag and size
        std::cout << "Atom [ " << atom.type[0] << atom.type[1] << atom.type[2] << atom.type[3] << " ] "<< real_size << " bytes" << std::endl;

        //
        // decode the FTYP atom
        //
        if( (atom.type[0] == 'f') && (atom.type[1] == 't') && (atom.type[2] == 'y') && (atom.type[3] == 'p') )
        {
            struct ftyp_t ftyp;
            if( fileNameProvided ) file.read( (char *)&ftyp, 8 );
            else std::cin.read( (char *)&ftyp, 8 );

            std::cout << "   Major Brand  : " << ftyp.majorBrand[0] << ftyp.majorBrand[1] << ftyp.majorBrand[2] << ftyp.majorBrand[3];
            std::cout << " v 0x" << std::hex << swapOrder(ftyp.minorVersion) << std::endl;

            // read the compatible formats, if they exist
            if( real_size > 16 )
            {
                buffer = new char[real_size-16];
                if( fileNameProvided ) file.read( buffer, real_size-16 );
                else std::cin.read( buffer, real_size-16 );

                std::cout << "   Compatible Brands : ";

                for( int i = 0; i < (real_size-16); i+=4 )
                {
                    std::cout << buffer[i] << buffer[i+1] << buffer[i+2] << buffer[i+3] << " ";
                }
                std::cout << std::endl;

                delete [] buffer;
            }

        //
        // Decode the MOOV atom
        //
        } else if( (atom.type[0] == 'm') && (atom.type[1] == 'o') && (atom.type[2] == 'o') && (atom.type[3] == 'v') )
        {
            // read the sub-atoms in the MOOV atom
            int remaining = real_size - 8;
            while( remaining > 0 )
            {
                struct atom_t moov_atom;
                if( fileNameProvided ) file.read( (char *)&moov_atom, 8 );
                else std::cin.read( (char *)&moov_atom, 8 );

                remaining -= 8;

                // display it
                int real_moov_size = swapOrder(moov_atom.size);
                std::cout << "   MOOV Atom [ " << moov_atom.type[0] << moov_atom.type[1] << moov_atom.type[2] << moov_atom.type[3] << " ] "<< real_moov_size << " bytes" << std::endl;

                unsigned char * moov_buffer = new unsigned char[real_moov_size-8];
                if( fileNameProvided ) file.read( (char *)moov_buffer, real_moov_size-8 );
                else std::cin.read( (char *)moov_buffer, real_moov_size-8 );

                delete [] moov_buffer;

                remaining -= real_moov_size-8;
            }

        } else {
            // read the rest of the atoms
            buffer = new char[real_size-8];
            if( fileNameProvided ) file.read( (char *)buffer, real_size-8 );
            else std::cin.read( (char *)buffer, real_size-8 );

            delete [] buffer;
        }

    }

    // close the file
    if( fileNameProvided ) file.close();

    std::cerr << std::endl << argv[0] << " : done" << std::endl;

    return 0;
}
