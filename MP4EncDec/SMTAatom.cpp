#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseSMTAatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding( m_depth );

    // this seems to be another container atom, but with a empty tag at the beginning    
    char * buffer;
    std::cout << std::endl;

    unsigned int reserved;
    file.read( (char*)&reserved, 4 );
    len -= 4;

    while( len > 0 )
    {
        struct atom_t atom;
        atom = getATOMhdr( file );
        len -= atom.bytes_read;

        // display it
        std::cout << padding << "[\033[1;32m" << atom.tag << "\033[0m] ";
        
        // check if this is a trak header (tkhd)
        if( atom.tag == "SAUT" ) parseSTRNGatom( file, atom.size );
        else if( atom.tag == "MDLN" ) parseSTRNGatom( file, atom.size );

        else parseUNKNatom( file, atom.size );
        
        len -= atom.size;
    }

    m_depth--;
}