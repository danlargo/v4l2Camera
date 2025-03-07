#include <fstream>
#include <map>
#include <string>
#include <iostream>

#include "walkMP4.h"

void parseATOM( std::ifstream &file, std::map<std::string, struct node_t*> dictionary, int a_size )
{
    m_depth++;

    struct atom_t atom;
    struct node_t * node;

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
            printATOMhdr( atom, node->description );
                        
            // check for nested atoms
            //if( atom.size > 0 )
            //{
                if( node->type == "ATOM_LIST" ) parseATOM( file, dictionary, (int)atom.size );
                else if( node->type == "BYTES" ) printFREEatom( file, atom );
                else if( node->type == "BINARY" ) printRAWatom( file, atom );
                else if( node->type == "CHARS" ) printCHARSatom( file, atom );
                else if( node->type == "STRUCT" ) parseSTRUCT( file, node, atom );

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

//
// Parse the STRUCT type
// - list of basic types
//
void parseSTRUCT( std::ifstream &file, struct node_t * node, struct atom_t atom )
{
    m_depth++;
    std::cout << calcPadding() << "  ";

    int remaining = atom.size;

    // walk the parts
    int ver = 0;

    for( auto it = node->parts.begin(); it != node->parts.end(); it++ )
    {
        struct node_t * n = it->second;

        if( n->type == "ATOM_LIST" ) parseATOM( file, node->parts, remaining );

        else if( n->type == "VER8" ) { ver = printVER8data( file, n->description ); remaining -= 1; }
        else if( n->type == "FLAGS" ) { printFLAGSdata( file, n->description, n->count ); remaining -= n->count; }

        else if( n->type == "INT8" ) { printINT8data( file, n->description ); remaining -= 1; }
        else if( n->type == "UINT8" ) { printUINT8data( file, n->description ); remaining -= 1; }

        else if( n->type == "INT16" ) { printINT16data( file, n->description ); remaining -= 2; }
        else if( n->type == "UINT16" ) { printUINT16data( file, n->description ); remaining -= 2; }

        else if( n->type == "INT32" ) { printINT32data( file, n->description ); remaining -= 4; }
        else if( n->type == "UINT32" ) { printUINT32data( file, n->description ); remaining -= 4; }
        
        else if( n->type == "HEX32" ) { printHEX32data( file, n->description ); remaining -= 4; }
        else if( n->type == "TAG4" ) { printTAG4data( file, n->description ); remaining -= 4; }
        
        else if( n->type == "INT16.2" ) { printINT1616data( file, n->description ); remaining -= 4; }
        else if( n->type == "INT8.2" ) { printINT88data( file, n->description ); remaining -= 2; }
        
        else if( n->type == "INT32COUNT" ) { printINT32COUNTdata( file, n->description, n->count ); remaining = (4*n->count); }
        else if( n->type == "TAG4_LIST" ) { printTAG4LISTdata( file, n->description, remaining ); remaining = 0; }
        
        else if( n->type == "RSVRD" ) { printRSRVDdata( file, n->description, n->count ); remaining -= (4*n->count); }
        else if( n->type == "MP4TIME" ) { int num = printMP4TIMEdata( file, n->description, ver ); remaining -= num; }

        else { printREMAININGdata( file, remaining ); remaining = 0; }
        if( remaining <= 0 ) break;
    }

    std::cout << std::endl;

    m_depth--;
}
//
// Parse the MP4 dictionary file
//
// - basically a JSON parser, or at least my interpretation of one
//
//
std::map<std::string, struct node_t*> parseDictionary( std::string fid )
{
    std::map<std::string, struct node_t*> ret;
    std::ifstream file;

    file.open( fid.c_str() );
    if( file.is_open() )
    {
        // figure out the size of the file
        file.seekg( 0, std::ios::end );
        int size = file.tellg();
        file.seekg( 0, std::ios::beg );

        if( size == 0 )
        {
            std::cerr << "[\033[1;31mfatal\033[0m] : dictionary file is empty" << std::endl;
            return ret;
        }

        // grab all the data at the same time
        char * buffer = new char[size];
        file.read( buffer, size );

        // close the file
        file.close();

        // parse the data
        std::string data = buffer;
        delete [] buffer;
        int offset = 0;

        // start at the beginning of the structure
        int start = data.find( "{", offset );
        if( start == -1 ) 
        {
            std::cerr << "[\033[1;31mfatal\033[0m] : dictionary file is empty or not json format" << std::endl;
            return ret;
        }

        // grab all the top level nodes
        struct node_t * n;
        offset = start +1;
        while( offset < data.length() )
        {
            n = getNode( data, offset );
            if( n->name != "" ) ret[n->name] = n;

            if( offset == -1 ) break;

            // jump over the comma
            if( data[offset] == ',' ) offset++;
        }

        file.close();
        // indicate success
        std::cerr << "[\033[1;34minfo\033[0m] : parsed " << ret.size() << " dictionary entries from - " << fid << std::endl;

    } else std::cerr << "[\033[1;31mfatal\033[0m] : could not open dictionary : " << fid << std::endl;

    return ret;

}

//
// Grab the entire node
//
// Simple Example :  "TOP" : { "Type" : "ATOM_LIST", "Description" : "Top Level of MP4/MOV/3GP file" },
//
// Nested Example :  "FTYP" : { "Type" : "STRUCT", "Description" : "File Type and Compatibility", "PARTS" : {} }
//
// - node names are not in top level searchable dictionary unless they are at the top level
// - nexted structures can be complex but can not be named, i.e. if an ATOM contains "stuff" and then an ATOM_LIST, the ATOM_LIST will be
//  considered a data element and should not have nested items
//
struct node_t * getNode( std::string buf, int &offset )
{
    struct node_t * n = new struct node_t;
    n->name = "";

    // Name will be from current offset to the first ':'
    int end = buf.find( ":", offset );
    if( end == -1 ) 
    {
        // couldn't find the name delimited by ':'
        offset = -1;
        return n;
    } else {
        // trim and remove the quotes
        n->name = buf.substr( offset, end-offset );
        trim( n->name );
        n->name = n->name.substr( 1, n->name.length()-2 );
        offset = end+1;

        // find the data structure, between matching {}
        n->raw_data = getRawData( buf, offset );
        trim( n->raw_data );

        // extract the TYPE, DESCRIPTION, COUNT and PARTS if they exist
        if( n->raw_data.length() > 0 )
        {
            n->type = getVal( n->raw_data, "Type", '"', '"' );
            std::string tmp = getVal( n->raw_data, "Count", '"', '"' );
            n->count = 1;
            if( tmp.length() > 0 )
            {
                try { n->count = std::stoi(tmp); }
                catch(const std::exception& e) { std::cerr << "Invalid count in dictionary : " << tmp << " " << e.what() << '\n'; }
            }
            n->description = getVal( n->raw_data, "Description", '"', '"' );
            n->raw_parts = getVal( n->raw_data, "Parts", '[', ']' );
        }

        // if there are parts, extract those
        if( n->raw_parts.length() > 0 ) n->parts = getParts( n->raw_parts);
        else n->parts.clear();
    }

    return n;
}

std::string getRawData( std::string buf, int &offset )
{
    std::string ret = "";

    int start = buf.find( "{", offset );
    if( start == -1 ) return ret;

    // walk the list until an equal number of { and } are found
    int count = 1;
    int index = start+1;

    while( (count > 0) && (index < buf.length()) )
    {
        if( buf[index] == '{' ) count++;
        if( buf[index] == '}' ) count--;

        index++;
    }

    // check if the bounds are equal
    if( count > 0 ) 
    {
        std::cout << "[\033[1;31mfatal\033[0m] : mismatched {} in token" << std::endl;
    }

    // return what we found
    offset = index;
    ret = buf.substr( start, index-start );

    return ret;
}

std::string getVal( std::string buf, std::string key, char start_del, char stop_del )
{
    std::string ret = "";
    int start = buf.find( key );

    if( start == -1 ) return ret;

    // find the value delimiter
    start = buf.find( ":", start );
    if( start == -1 ) 
    {
        std::cout << "[\033[1;31mfatal\033[0m] : colon required after key, before value : " << key << std::endl;
        return ret;
    }

    // find the value
    start = buf.find( start_del, start+1 );
    if( start == -1 ) 
    {
        std::cout << "[\033[1;31mfatal\033[0m] : could not find start delimiter for key : " << key << std::endl;
        return ret;
    }

    // find the end of the value
    int end = buf.find( stop_del, start+1 );
    if( end == -1 )
    {
        std::cout << "[\033[1;31mfatal\033[0m] : could not find end delimiter for key : " << key << std::endl;
        return ret;
    }

    // extract the value
    ret = buf.substr( start+1, end-start-1 );
    trim( ret );

    return ret;
}

std::map<std::string, struct node_t*> getParts( std::string buf )
{
    std::map<std::string, struct node_t*> ret;
    int offset = 0;
    int count = 1;

    // parts will be between curly braces
    while( offset < buf.length() )
    {
        int start = buf.find( "{", offset );
        if( start == -1 ) break;

        int end = buf.find( "}", start );
        if( end == -1 ) break;
        offset = end+1;

        // get the parts
        struct node_t * n = new struct node_t;

        // make sure the parts stay sorted
        if( count < 10 ) n->name = "part0" + std::to_string(count++);
        else n->name = "part" + std::to_string(count++);

        n->raw_data = buf.substr( start, end-start+1 );
        trim( n->raw_data );

        // get the type and description
        n->type = getVal( n->raw_data, "Type", '"', '"' );
        std::string tmp = getVal( n->raw_data, "Count", '"', '"' );
        n->count = 1;
        if( tmp.length() > 0 )
        {
            try { n->count = std::stoi(tmp); }
            catch(const std::exception& e) { std::cerr << "Invalid count in dictionary : " << tmp << " " << e.what() << '\n'; }
        }
        n->description = getVal( n->raw_data, "Description", '"', '"' );

        ret[n->name] = n;
    }

    return ret;
}