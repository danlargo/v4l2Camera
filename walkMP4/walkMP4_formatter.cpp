#include <fstream>
#include <map>
#include <string>
#include <iostream>
#include <iomanip>

#include "walkMP4.h"


void printATOMhdr( struct atom_t atom, std::string descrip )
{
    std::cout << calcPadding();
    trim(descrip);
    if( descrip.length() > 0 ) std::cout << "[\033[1;32m" << atom.tag << "\033[0m] \033[1;37m" << descrip << "\033[0m" << std::endl;
    else 
    {
        std::cout.imbue(std::locale(""));
        std::cout << std::dec << "[\033[1;32m" << atom.tag << "\033[0m] \033[1;33m(" << atom.size << " bytes)  ...not in dictionary\033[0m" << std::endl;
        std::locale::classic();
    }

}


void printFREEatom( std::ifstream &file, struct atom_t atom )
{
    m_depth++;

    int dump_len = 64;
    // we can grab this all at once as it is not likely to be fucking huge
    char * buffer = new char[atom.size];
    file.read( buffer, atom.size );

    // dump up to 80 chars that we find in the data
    std::cout << std::dec << calcPadding();

    std::cout << "  (" << atom.size << " bytes total)";

    if( atom.size > 0 )
    {
        // now dump the raw bytes
        std::cout << ", (first " << dump_len << " bytes) : ";
        for( int i = 0; i < (dump_len/2); i++ )
        {
            if( i >= atom.size ) break;

            std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i] << " ";
        }
    }

    delete [] buffer;

    std::cout << std::endl;

    m_depth--;
}

void printRAWatom( std::ifstream &file, struct atom_t atom )
{
    m_depth++;

    int chunk = 2048;
    int grab_size = atom.size;

    std::cout.imbue(std::locale(""));
    std::cout << calcPadding() << std::dec << "  (binary data) " << atom.size << " bytes" << std::endl;
    std::locale::classic();

    // grab a chunk at a time as it could be gigabytes long
    while( grab_size > 0 )
    {
        char * buffer = new char[chunk];
        int read_size = (grab_size > chunk) ? chunk : grab_size;
        file.read( buffer, read_size );

        grab_size -= read_size;
        delete [] buffer;
    }

    m_depth--;
}


void printCHARSatom( std::ifstream &file, struct atom_t atom )
{
    m_depth++;

    std::cout.imbue(std::locale(""));
    std::cout << calcPadding() << std::dec << "  (chars) " << atom.size << " bytes";
    std::locale::classic();

    char * buffer = new char[atom.size+1];
    file.read( buffer, atom.size );
    buffer[atom.size] = 0;

    std::cout << " : \033[0;36m" << buffer << "\033[0m" << std::endl;

    delete [] buffer;

    m_depth--;
}

void printUNKNatom( std::ifstream &file, struct atom_t atom )
{
    m_depth++;
    int chunk = 16;
    int num_lines = 2;

    int grab_size = atom.size;

    // grab a chunk at a time as it could be gigabytes long
    while( grab_size > 0 )
    {
        char * buffer = new char[chunk];
        int read_size = (grab_size > chunk) ? chunk : grab_size;
        file.read( buffer, read_size );

        // display it in a grid
        if( num_lines > 0 )
        {
            std::cout << calcPadding() << "  ";
            for( int i = 0; i < read_size; i++ )
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i] << " ";
            }
            // print as ascii
            for( int i = 0; i < read_size; i++ )
            {
                if( isprint(buffer[i]) ) std::cout << buffer[i];
                else std::cout << ".";
            }
            std::cout << std::endl;

            num_lines--;
        }

        grab_size -= read_size;
        delete [] buffer;
    }

    m_depth--;
}


std::string calcPadding()
{
    std::string ret = "";
    for( int i = 1; i < m_depth; i++ ) ret += "     ";

    return ret;
}

void printDictionary( std::map<std::string, struct node_t> dictionary )
{
    std::map<std::string, struct node_t>::iterator it;

    std::cout << std::endl  << "walkMP4 Data Dictionary" << std::endl;
    std::cout               << "=======================" << std::endl;

    for( it = dictionary.begin(); it != dictionary.end(); it++ )
    {
        std::cout << it->first << " : " << it->second.type << " : " << it->second.description << std::endl;
    }

    std::cout << std::endl << "...done print" << std::endl;
}