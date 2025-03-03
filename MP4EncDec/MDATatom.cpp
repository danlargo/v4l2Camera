#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <limits>

#include "decodeMP4.h"

void parseMDATatom(  std::ifstream &file, unsigned long len )
{
    m_depth++;

    if (len > static_cast<unsigned long>(std::numeric_limits<std::streamsize>::max())) {
        std::cerr << "Error: Length exceeds maximum value for std::streamsize." << std::endl;
        return;
    }

    std::streamsize slen = static_cast<std::streamsize>(len);
    
    // this is free data atom
    char * buffer;

    // display the len
    std::cout.imbue(std::locale(""));

    std::cout << std::dec << "Raw Media Data : " << len << " bytes " << std::endl;

    buffer = new char[len];
    file.read( buffer, slen );

    delete [] buffer;

    // add a blank line
    std::cout << std::endl;

    m_depth--;
}