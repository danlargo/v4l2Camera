#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseMETAatom(  std::ifstream &file, unsigned long len, bool checkVersion )
{
    m_depth++;
    std::string padding = calcPadding(m_depth);

    // depending on where this is located there might be version information
    if( checkVersion )
    {
        struct version_t ver;

        file.read( (char *)&ver , 4 );
        std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">" << std::endl;
        len -= 4;

    } else std::cout << std::endl;


    // this is a container atom
    char * buffer;

    while( len > 0 )
    {
        struct atom_t atom;
        atom = getATOMhdr( file );

        // display it
        if( checkVersion ) padding += "      ";

        std::cout << padding << "[\033[1;32m";
        std::cout << atom.tag << "\033[0m] ";
        
        len -= 8;

        if( atom.tag == "HDLR" ) parseHDLRatom( file, atom.size );
        else if( atom.tag == "ILST" ) parseILSTatom( file, atom.size );
        else if( atom.tag == "FREE" ) parseFREEatom( file, atom.size );
        else if( atom.tag == "KEYS" ) parseKEYSatom( file, atom.size );

        else parseUNKNatom( file, atom.size );

        len -= atom.size;
    }

    m_depth--;

}