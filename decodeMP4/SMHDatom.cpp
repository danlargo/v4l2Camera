#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseSMHDatom( std::ifstream &file, unsigned long len )
{
    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">,";

    len -= 4;

    unsigned char balance[2];
    file.read( (char *)&balance, 2 );
    len -= 2;

    unsigned short reserved;
    file.read( (char *)&reserved, 2 );
    len -= 2;

    std::cout << " sound balance <left " << (int)balance[0] << ":" << (int)balance[1] << " right>" << std::endl;

}