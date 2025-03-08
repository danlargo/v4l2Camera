#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseMINFatom( std::ifstream &file, unsigned long len )
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
        if( atom.tag == "DINF" ) parseDINFatom( file, atom.size );
        else if( atom.tag == "SMHD" ) parseSMHDatom( file, atom.size );
        else if( atom.tag == "VMHD" ) parseVMHDatom( file, atom.size );
        else if( atom.tag == "GMHD" ) parseGMHDatom( file, atom.size );
        else if( atom.tag == "HDLR" ) parseHDLRatom( file, atom.size );
        else if( atom.tag == "STBL" ) parseSTBLatom( file, atom.size );

        else parseUNKNatom( file, atom.size );

        len -= atom.size;
    }

    m_depth--;
}