#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseSTBLatom(  std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    // this is a container atom
    char * buffer;
    std::cout << std::endl;

    while( len > 0 )
    {
        struct atom_t atom;
        atom = getATOMhdr( file );

        // display it
        std::cout << padding << "[\033[1;32m" << atom.tag << "\033[0m] ";
        
        len -= 8;

        if( atom.tag == "FREE" ) parseFREEatom( file, atom.size );
        else if( atom.tag == "META" ) parseMETAatom( file, atom.size, true );
        else if( atom.tag == "STSD" ) parseSTSDatom( file, atom.size );
        else if( atom.tag == "STTS" ) parseSTTSatom( file, atom.size );
        else if( atom.tag == "STSC" ) parseSTSCatom( file, atom.size );

        else parseUNKNatom( file, atom.size );
        
        len -= atom.size;
    }

    m_depth--;
}