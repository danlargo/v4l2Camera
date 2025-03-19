#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseAUTHatom( std::ifstream &file, unsigned long len )
{

    // Strucutre of the atom
    //
    //  Delimiter: 4 bytes (seems to be all zeroes)
    //
    //  Language: 2 bytes (ISO 639-2/T language code)
    //
    //  Author information: variable length string (maybe zero terminated)
    //

    unsigned int size = len;

    // read the delimiter
    char buffer[4];
    file.read( buffer, 4 );
    size -= 4;

    // read the language code
    unsigned short lang;
    file.read( (char*)&lang, 2 );
    size -= 2;

    std::string language = decode_lang( swapShort(lang) );

    // read the author information, zero terminated
    std::string author;
    char c;
    while( file.get(c) && c != 0 && size > 1)
    {
        author += c;
        size --;
    }

    // display the inforation
    std::cout << len << " bytes";

    std::cout << ", lang <" << language << ">, author [\033[1;33m" << author << "\033[0m]" << std::endl;

}