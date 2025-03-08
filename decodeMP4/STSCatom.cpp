#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseSTSCatom( std::ifstream &file, unsigned long len )
{
    struct version_t ver;

    file.read( (char *)&ver , 4 );
    std::cout << "v" << (int)ver.version << ", flags <" << std::hex << (int)ver.flags[0] << (int)ver.flags[1] << (int)ver.flags[2] << std::dec << ">,";

    len -= 4;

    // now parse the rest of the header
    //
    //  Version (1 byte): Typically 0x00 (version 0 is standard; version 1 uses larger fields, rare).
    //
    //  Flags (3 bytes): Usually 0x000000 (reserved, no defined flags).
    //
    //  Payload
    //      Entry Count (4 bytes): A 32-bit unsigned integer specifying the number of entries in the table that follows. Each entry describes a run of consecutive chunks with the same number of samples.
    //
    //      Chunk Table (variable length): An array of entries, where each entry consists of:
    //          First Chunk (4 bytes): The index of the first chunk in this run (1-based, not 0-based).
    //
    //          Samples per Chunk (4 bytes): Number of samples in each chunk of this run.
    // 
    //          Sample Description Index (4 bytes): Index into the stsd table (usually 0x00000001 for a single codec).
    //
    unsigned int count;
    file.read( (char *)&count, 4 );
    len -= 4;

    std::cout << " count <" << swapOrder(count) << ">";

    unsigned int first_chunk;
    file.read( (char *)&first_chunk, 4 );
    len -= 4;
    first_chunk = swapOrder(first_chunk);

    unsigned int samples;
    file.read( (char *)&samples, 4 );
    len -= 4;
    samples = swapOrder(samples);

    unsigned int index;
    file.read( (char *)&index, 4 );
    len -= 4;
    index = swapOrder(index);

    std::cout << "  first chunk <" << first_chunk << ">, samples <" << samples << " per chunk>, index<" << index << ">" << std::endl;

    // check for spare data
    if( len > 0 )
    {
        unsigned char * fld = new unsigned char[len];
        file.read( (char *)fld, len);
        delete [] fld;
    }
        
}

