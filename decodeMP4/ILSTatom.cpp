#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseILSTatom(  std::ifstream &file, unsigned long len )
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

        // check if the first character is 0xA( (copyright character)
        if( (unsigned char)atom.tag[0] == 0xA9 ) parseCOPYatom( file, atom );
        else if( atom.tag.length() == 0 ) parseEMPTYatom( file, atom );

        else 
        {        
            std::cout << padding << "[\033[1;32m" << atom.tag << "\033[0m] ";
            parseUNKNatom( file, atom.size );
        }

        len -= atom.size;
    }
    m_depth--;
}