#include <fstream>
#include <map>
#include <string>
#include <iostream>

#include "walkMP4.h"

void parseATOM( std::ifstream &file, std::map<std::string, struct node_t> dictionary, int a_size )
{
    m_depth++;

    struct atom_t atom;
    struct node_t node;

    while( !file.eof() && ((a_size == -1) || (a_size > 0)) )
    {
        atom = readATOM( file );
        if( atom.tag == "" ) break;

        // account for data consumed at next level
        if( a_size != -1 ) a_size -= atom.size+8;

        // check if we have a dictionary entry
        if( dictionary.find( atom.tag ) != dictionary.end() )
        {
            node = dictionary[atom.tag];
            printATOMhdr( atom, node.description );
                        
            // check for nested atoms
            //if( atom.size > 0 )
            //{
                if( node.type == "ATOM_LIST" ) parseATOM( file, dictionary, (int)atom.size );
                else if( node.type == "BYTES" ) printFREEatom( file, atom );
                else if( node.type == "BINARY" ) printRAWatom( file, atom );
                else if( node.type == "CHARS" ) printCHARSatom( file, atom );

                else printUNKNatom( file, atom );

            //}

        } else {
            printATOMhdr( atom, "" );
            printUNKNatom( file, atom );
        }

    }

    m_depth--;
}

struct atom_t readATOM( std::ifstream &file )
{
    struct atom_t atom;

    atom.size = 0;
    atom.bytes_read = 0;
    atom.tag = "";

    unsigned int size;
    file.read( (char *)&size, 4 );
    atom.bytes_read = 4;

    // check EOF
    if( file.eof() ) return atom;

    unsigned int real_size = swapEndian(size);

    atom.orig[4] = 0;
    file.read( (char *)&atom.orig, 4 );
    atom.bytes_read += 4;

    std::string tmp = (char *)atom.orig;
    atom.tag = toUpper(tmp);

    if( real_size == 1 )
    {
        // read the extended size
        unsigned long ext_size;
        file.read( (char *)&ext_size, 8 );
        atom.bytes_read += 8;

        atom.size = swapEndian(ext_size) - 16;

    } else atom.size = real_size - 8;

    return atom;
}


std::map<std::string, struct node_t> parseDictionary( std::string fid )
{
    std::map<std::string, struct node_t> ret = {};

    std::ifstream file;
    std::string line;

    int pos;
    int count = 0;

    file.open( fid.c_str() );
    if( file.is_open() )
    {
        while( !file.eof() )
        {
            getline( file, line );
            trim(line);
            if( line.length() == 0 ) continue;

            // parse the line
            int start = line.find( "{" );
            if( start > -1 )
            {
                struct node_t n;
                n.parts = {};;

                n.name = line.substr( 0, start );
                n.name = toUpper( n.name );
                n.name = n.name.substr( 0, n.name.length() - 1 );

                int end = line.find( "}" );
                if( end == -1 )
                {
                    end = line.find( "[" );
                }

                n.type = line.substr( start+1, end - start - 1 );
                trim(n.type);
                n.type = toUpper( n.type );

                if( n.type == "PARTS" )
                {
                    // we need to find all the sub parts
                }

                n.description = line.substr( line.find( "}" ) + 1, line.length() - line.find( "}" ) - 1 );
                trim(n.description);
                ret[n.name] = n;
            }

            count++;
        }
        file.close();
        // indicate success
        std::cerr << "[\033[1;34minfo\033[0m] : parsed " << count << " dictionary entries from - " << fid << std::endl;

    } else std::cerr << "[\033[1;31mfatal\033[0m] : could not open dictionary : " << fid << std::endl;

    return ret;

}
