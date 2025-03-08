#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseQTCOPYatom( std::ifstream &file, struct atom_t atom )
{
    std::string padding = calcPadding(m_depth);

    // fix up the copy right symbol
    atom.tag = "(c)" + atom.tag.substr(1);
    unsigned int len = atom.size;
    char * buffer;

    std::cout << padding << "[\033[1;32m" << atom.tag << "\033[0m] " << std::dec << std::setw(2) << std::setfill('0') << atom.size << " bytes : ";
    
    // apply a label to the atom (could be MOV, SWR, DAY, XYZ, MAK)
    if( atom.tag == "(c)MOD" ) std::cout << "Model      : ";
    if( atom.tag == "(c)SWR" ) std::cout << "Version    : ";
    if( atom.tag == "(c)DAY" ) std::cout << "Date       : ";
    if( atom.tag == "(c)XYZ" ) std::cout << "Location   : ";
    if( atom.tag == "(c)MAK" ) std::cout << "Make       : ";

    // process the atom data
    //
    //  Normal Atom Structure (above)
    //      Size (2 bytes): Size of the item atom (header + payload).
    //      
    //      Language Code (2 bytes) : ISO-639-2/T language code.
    //
    //      Data (variable length): The actual value (e.g., string, integer), length = Size - 4
    //          not zero terminated.

    unsigned short p_size;
    file.read( (char *)&p_size, 2 );
    len -= 2;

    // get the language code
    unsigned short lang;
    file.read( (char*)&lang, 2 );
    len -= 2;

    std::string langStr = decode_lang(swapShort(lang));

    // get the payload
    buffer = new char[swapShort(p_size)+1];
    file.read( buffer, swapShort(p_size) );
    buffer[swapShort(p_size)] = 0;
    len -= swapShort(p_size);

    // display the info
    std::cout << " <" << langStr << "> [\033[1;33m" << buffer << "\033[0m]" << std::endl;

    // delete the buffer
    delete [] buffer;
    
}