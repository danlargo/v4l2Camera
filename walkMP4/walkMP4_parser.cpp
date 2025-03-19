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
            if( node->type == "ATOM_LIST" ) parseATOM( file, dictionary, (int)atom.size );

            else if( node->type == "BYTES" ) printFREEdata( file, atom.size );
            else if( node->type == "BINARY" ) printRAWdata( file, atom.size );
            else if( node->type == "CHARS" ) printCHARSdata( file, atom.size );
            else if( node->type == "STRUCT" ) parseSTRUCT( file, dictionary, node, atom.size );
            else if( node->type == "LIST" ) parseLISTdata( file, dictionary, node, atom.size );

            else printUNKNdata( file, atom );

        } else {
            printATOMhdr( atom, "" );
            printUNKNdata( file, atom );
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

    // check if there is a copyright symbol
    std::string tmp = "";
    std::string tmp2 = (char*)atom.orig;
    if( atom.orig[0] == 0xA9 ) 
    {
        tmp = "(c)" + toUpper(tmp2.substr(1, 3));
    }
    else tmp = toUpper(tmp2);

    atom.tag = tmp;
    trim(atom.tag);

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
void parseSTRUCT( std::ifstream &file, std::map<std::string, struct node_t *> dictionary, struct node_t * node, int remaining )
{
    m_depth++;
    m_struct_depth++;

    std::cout << calcPadding() << " ";

    // walk the parts
    int ver = 0;

    for( auto it = node->parts.begin(); it != node->parts.end(); it++ )
    {
        struct node_t * n = *it;

        if( n->type == "ATOM_LIST" ) { std::cout << std::endl; parseATOM( file, dictionary, remaining ); }

        else if( n->type == "LIST" ) { parseLISTdata( file, dictionary, n, remaining ); remaining = 0; }

        else if( n->type == "VER8" ) { ver = printVER8data( file, n ); remaining -= 1; }
        else if( n->type == "FLAGS" ) { printFLAGSdata( file, n ); remaining -= n->count; }

        else if( n->type == "INT8" ) { printINTEGERdata( file, n, false, false, 1 ); remaining -= 1; }
        else if( n->type == "UINT8" ) { printINTEGERdata( file, n, true, false, 1 ); remaining -= 1; }

        else if( n->type == "INT16" ) { printINTEGERdata( file, n, false, false, 2 ); remaining -= 2; }
        else if( n->type == "UINT16" ) { printINTEGERdata( file, n, true, false, 2 ); remaining -= 2; }

        else if( n->type == "INT32" ) { printINTEGERdata( file, n, false, false, 4 ); remaining -= 4; }
        else if( n->type == "UINT32" ) { printINTEGERdata( file, n, true, false, 4 ); remaining -= 4; }
        
        else if( n->type == "HEX8" ) { printINTEGERdata( file, n, true, true, 1 ); remaining -= 1; }
        else if( n->type == "HEX16" ) { printINTEGERdata( file, n, true, true, 2 ); remaining -= 2; }
        else if( n->type == "HEX32" ) { printINTEGERdata( file, n, true, true, 4 ); remaining -= 4; }

        else if( n->type == "LANG16" ) { printLANGdata( file, n ); remaining -= 2; }

        else if( n->type == "TAG4" ) { printTAG4data( file, n ); remaining -= 4; }
        else if( n->type == "TAG4_LIST" ) { printTAG4LISTdata( file, n, remaining ); remaining = 0; }

        else if( n->type == "INT16.2" ) { printINT1616data( file, n, false ); remaining -= 4; }
        else if( n->type == "UINT16.2" ) { printINT1616data( file, n, true ); remaining -= 4; }
        else if( n->type == "INT8.2" ) { printINT88data( file, n, false ); remaining -= 2; }
        else if( n->type == "UINT8.2" ) { printINT88data( file, n, true ); remaining -= 2; }
        
        else if( n->type == "IGNORE" ) { skipIGNOREdata( file, n->count ); remaining -= n->count; }
        else if( n->type == "MP4TIME" ) { int num = printMP4TIMEdata( file, n, ver ); remaining -= num; }
        else if( n->type == "MP4TICKS" ) { int num = printMP4TICKSdata( file, n, ver ); remaining -= num; }

        else if( n->type == "MATH_DIV" ) { printMATHdata( n ); }

        else if( n->type == "STRING" ) { int num = printSTRINGdata( file, n, remaining ); remaining -= num; }

        else if( n->type == "NEWLINE" ) { printFORMAT( n ); }
        else if( n->type == "LABEL" ) { printFORMAT( n ); }

        else if( node->type == "BYTES" ) { printFREEdata( file, remaining ); remaining = 0; }
        else if( node->type == "BINARY" ) { printRAWdata( file, remaining ); remaining = 0; }
        else if( node->type == "CHARS" ) { printCHARSdata( file, remaining ); remaining = 0; }

        else { printREMAININGdata( file, remaining ); remaining = 0; }
        
        if( remaining <= 0 ) break;
    }

    if( m_struct_depth == 1 ) std::cout << std::endl;

    m_depth--;
    m_struct_depth--;
}

//
// Parse the LIST type
// - list of basic types, repeated n-times
//
void parseLISTdata( std::ifstream &file, std::map<std::string, struct node_t *> dictionary, struct node_t * node, int remaining )
{
    m_depth++;
    m_struct_depth++;

    std::cout << std::endl << calcPadding() << "List of : \033[1;37m" << node->description << "\033[0m" << std::endl;

    // walk the parts
    int ver = 0;
    int count = 1;

    while( remaining > 0 )
    {
        std::cout << calcPadding() << "[\033[1;32m" << count++ << "\033[0m]";

        for( auto it = node->parts.begin(); it != node->parts.end(); it++ )
        {
            struct node_t * n = *it;

            if( n->type == "ATOM_LIST" ) { std::cout << std::endl; parseATOM( file, dictionary, remaining ); }

            else if( n->type == "VER8" ) { ver = printVER8data( file, n ); remaining -= 1; }
            else if( n->type == "FLAGS" ) { printFLAGSdata( file, n ); remaining -= n->count; }

            else if( n->type == "INT8" ) { printINTEGERdata( file, n, false, false, 1 ); remaining -= 1; }
            else if( n->type == "UINT8" ) { printINTEGERdata( file, n, true, false, 1 ); remaining -= 1; }

            else if( n->type == "INT16" ) { printINTEGERdata( file, n, false, false, 2 ); remaining -= 2; }
            else if( n->type == "UINT16" ) { printINTEGERdata( file, n, true, false, 2 ); remaining -= 2; }

            else if( n->type == "INT32" ) { printINTEGERdata( file, n, false, false, 4 ); remaining -= 4; }
            else if( n->type == "UINT32" ) { printINTEGERdata( file, n, true, false, 4 ); remaining -= 4; }
            
            else if( n->type == "HEX8" ) { printINTEGERdata( file, n, true, true, 1 ); remaining -= 1; }
            else if( n->type == "HEX16" ) { printINTEGERdata( file, n, true, true, 2 ); remaining -= 2; }
            else if( n->type == "HEX32" ) { printINTEGERdata( file, n, true, true, 4 ); remaining -= 4; }

            else if( n->type == "LANG16" ) { printLANGdata( file, n ); remaining -= 2; }

            else if( n->type == "TAG4" ) { printTAG4data( file, n ); remaining -= 4; }
            else if( n->type == "TAG4_LIST" ) { printTAG4LISTdata( file, n, remaining ); remaining = 0; }

            else if( n->type == "INT16.2" ) { printINT1616data( file, n, false ); remaining -= 4; }
            else if( n->type == "UINT16.2" ) { printINT1616data( file, n, true ); remaining -= 4; }
            else if( n->type == "INT8.2" ) { printINT88data( file, n, false ); remaining -= 2; }
            else if( n->type == "UINT8.2" ) { printINT88data( file, n, true ); remaining -= 2; }
            
            else if( n->type == "IGNORE" ) { skipIGNOREdata( file, n->count ); remaining -= n->count; }
            else if( n->type == "MP4TIME" ) { int num = printMP4TIMEdata( file, n, ver ); remaining -= num; }
            else if( n->type == "MP4TICKS" ) { int num = printMP4TICKSdata( file, n, ver ); remaining -= num; }

            else if( n->type == "MATH_DIV" ) { printMATHdata( n ); }

            else if( n->type == "STRING" ) { int num = printSTRINGdata( file, n, remaining ); remaining -= num; }

            else if( n->type == "NEWLINE" ) { printFORMAT( n ); }
            else if( n->type == "LABEL" ) { printFORMAT( n ); }

            else { printREMAININGdata( file, remaining ); remaining = 0; }
            if( remaining <= 0 ) break;
        }

        std::cout << std::endl;
    }

    if( m_struct_depth == 1 ) std::cout << std::endl;

    m_depth--;
    m_struct_depth--;
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
            std::cerr << "[\033[1;31mfatal\033[0m] : dictionary file is empty or not correct format" << std::endl;
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

        // close the dictionary file
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

    // split the data between the name and the data, which is delimited by matching braces {}
    n->name = getNodeName( buf, offset );

    if( n->name.length() > 0 )
    {   
        // find the data structure, between matching {}
        n->raw_data = getNodeData( buf, offset );

        // extract the TYPE, DESCRIPTION, COUNT and PARTS if they exist
        std::string inner = "";
        if( n->raw_data.length() > 0 )
        {
            inner = getInner( n->raw_data );
            n->raw_data = removeInner( n->raw_data );

            // now try to find the data elements
            n->type = getVal( n->raw_data, "Type", '"', '"' );
            std::string tmp = getVal( n->raw_data, "Count", '"', '"' );
            n->count = 1;
            if( tmp.length() > 0 )
            {
                try { n->count = std::stoi(tmp); }
                catch(const std::exception& e) { std::cerr << "Invalid count in dictionary : " << tmp << " " << e.what() << '\n'; }
            }
            n->description = getVal( n->raw_data, "Description", '"', '"' );
            n->units = getVal( n->raw_data, "Units", '"', '"' );
            n->var1 = getVal( n->raw_data, "Var1", '"', '"' );
            n->var2 = getVal( n->raw_data, "Var2", '"', '"' );
        }

        // if there are parts, extract those
        if( inner.length() > 0 ) n->parts = getParts( inner );
        else n->parts.clear();
    }

    return n;
}

std::string getNodeName( std::string buf, int &offset )
{
    std::string ret = "";

    // Name will be from current offset to the first ':'
    int end = buf.find( ":", offset );
    if( end == -1 ) 
    {
        // couldn't find the name delimited by ':'
        offset = -1;
        return "";
    } else {
        // trim the name
        ret = buf.substr( offset, end-offset );
        trim( ret );
        // remove the quotes
        ret = ret.substr( 1, ret.length()-2 );
        offset = end+1;
    }

    return ret;
}

std::string getNodeData( std::string buf, int &offset )
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

    // push the parsing offset forward
    offset = index+1;

    // return what we found, minus the braces
    ret = buf.substr( start+1, index-start-2 );
    trim( ret );

    return ret;
}

std::string getInner( std::string in )
{
    int offset = in.find( "[" );
    if( offset == -1 ) return "";

    int offset2 = in.find_last_of( "]" );
    if( offset2 == -1 ) return "";

    std::string out = in.substr( offset+1, offset2-offset-1 );
    trim(out);

    return out;
}

std::string removeInner( std::string in )
{
    int offset = in.find( "[" );
    if( offset == -1 ) return in;

    int offset2 = in.find_last_of( "]" );
    if( offset2 == -1 ) return in;

    std::string out = in.substr( 0, offset );
    out += in.substr( offset2+1 );

    return out;

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

std::vector<struct node_t*> getParts( std::string buf )
{
    std::vector<struct node_t*> ret;
    int offset = 0;
    int count = 1;

    // parts will be between curly braces
    while( offset < buf.length() )
    {
        // get the parts
        struct node_t * n = new struct node_t;

        // make sure the parts stay sorted
        if( count < 10 ) n->name = "part0" + std::to_string(count++);
        else n->name = "part" + std::to_string(count++);

        n->raw_data = getNodeData( buf, offset );

        // remove the inner data so we don't process it here
        std::string inner = getInner( n->raw_data );
        n->raw_data = removeInner( n->raw_data );

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
        n->units = getVal( n->raw_data, "Units", '"', '"' );
        n->var1 = getVal( n->raw_data, "Var1", '"', '"' );
        n->var2 = getVal( n->raw_data, "Var2", '"', '"' );

        // if there are parts, extract those
        if( inner.length() > 0 ) n->parts = getParts( inner );
        else n->parts.clear();

        // add to the list
        ret.push_back(n);
    }

    return ret;
}