#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseUDTAatom(  std::ifstream &file, unsigned long len )
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

        if( (unsigned char)atom.tag[0] == 0xA9 ) parseQTCOPYatom( file, atom );
        else 
        {
            // display the atom tag
            std::cout << padding << "[\033[1;32m" << atom.tag << "\033[0m] ";

            if( atom.tag == "FREE" ) parseFREEatom( file, atom.size );
            else if( atom.tag == "META" ) parseMETAatom( file, atom.size, true );
            else if( atom.tag == "SDLN" ) parseSTRNGatom( file, atom.size );
            else if( atom.tag == "SMRD" ) parseSTRNGatom( file, atom.size );
            else if( atom.tag == "FIRM" ) parseSTRNGatom( file, atom.size );
            else if( atom.tag == "LENS" ) parseSTRNGatom( file, atom.size );
            else if( atom.tag == "AUTH" ) parseAUTHatom( file, atom.size );
            else if( atom.tag == "SMTA" ) parseSMTAatom( file, atom.size );
            else if( atom.tag == "CAME" ) parseRAWatom( file, atom.size );
            else if( atom.tag == "SETT" ) parseRAWatom( file, atom.size );
            else if( atom.tag == "MUID" ) parseRAWatom( file, atom.size );
            else if( atom.tag == "BCID" ) parseRAWatom( file, atom.size );
            else if( atom.tag == "GUMI" ) parseRAWatom( file, atom.size );
            else if( atom.tag == "HMMT" ) parseFREEatom( file, atom.size );
            else if( atom.tag == "GPMF" ) parseGPMFatom( file, atom.size );

            else parseUNKNatom( file, atom.size );

        }

        len -= atom.size;
    }
    m_depth--;
}