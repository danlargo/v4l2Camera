#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

// MOOV atom
//
void parseMOOVatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    // read the sub-atoms in the MOOV atom
    char * buffer;
    std::cout << std::endl;

    while( len > 0 )
    {
        struct atom_t atom;
        atom = getATOMhdr( file );
        len -= atom.bytes_read;

        // display it
        std::cout << padding << "[\033[1;32m" << atom.tag << "\033[0m] ";
        
        if( atom.tag == "MVHD" ) parseMVHDatom( file, atom.size );
        else if( atom.tag == "TRAK" ) parseTRAKatom( file, atom.size );
        else if( atom.tag == "IODS" ) parseIODSatom( file, atom.size );
        else if( atom.tag == "UDTA" ) parseUDTAatom( file, atom.size );
        else if( atom.tag == "FREE" ) parseFREEatom( file, atom.size );
        else if( atom.tag == "META" ) parseMETAatom( file, atom.size, false);

        else parseUNKNatom( file, atom.size );

        len -= atom.size;

    }

    // add a blank line
    std::cout << std::endl;

    m_depth--;
}