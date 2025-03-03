#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseEDTSatom( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    // this is another container atom
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
        if( atom.tag == "ELST" ) parseELSTatom( file, atom.size );
        else {
            std::cout << "not parsed yet (" << atom.size << " bytes)" << std::endl;

            // discard the atom
            buffer = new char[atom.size];
            file.read( buffer, atom.size );
            delete [] buffer;
        }
        len -= atom.size;
    }
    m_depth--;
}