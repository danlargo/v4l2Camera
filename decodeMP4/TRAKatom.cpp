#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

// TRAK atom
//
void parseTRAKatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    // read the sub-atoms in the TRAK atom
    char * buffer;
    std::cout << std::endl;

    while( len > 0 )
    {
        struct atom_t atom;
        atom = getATOMhdr( file );
        len -= atom.bytes_read;
        
        // display it
        std::cout << padding << "[\033[1;32m" << atom.tag << "\033[0m] ";
        
        // check if this is a trak header (tkhd)
        if( atom.tag == "TKHD" ) parseTKHDatom( file, atom.size );
        else if( atom.tag == "MDIA" ) parseMDIAatom( file, atom.size );
        else if( atom.tag == "EDTS" ) parseEDTSatom( file, atom.size );
        else if( atom.tag == "TREF" ) parseTREFatom( file, atom.size );
        else if( atom.tag == "TAPT" ) parseTAPTatom( file, atom.size );
        else if( atom.tag == "META" ) parseMETAatom( file, atom.size, false );
        
        else parseUNKNatom( file, atom.size );

        len -= atom.size;
    }

    m_depth--;
}