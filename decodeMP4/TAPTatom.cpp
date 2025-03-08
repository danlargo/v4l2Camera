#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseTAPTatom(  std::ifstream &file, unsigned long len )
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
        len -= 8;

        // display the atom tag
        std::cout << padding << "[\033[1;32m" << atom.tag << "\033[0m] ";

        if( atom.tag == "FREE" ) parseFREEatom( file, atom.size );
        else if( atom.tag == "CLEF") parseRAWatom( file, atom.size );
        else if( atom.tag == "PROF") parseRAWatom( file, atom.size );
        else if( atom.tag == "ENOF") parseRAWatom( file, atom.size );

        else parseUNKNatom( file, atom.size );

        len -= atom.size;
    }
    m_depth--;
}