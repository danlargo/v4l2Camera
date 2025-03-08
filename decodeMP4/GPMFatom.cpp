#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "decodeMP4.h"

void parseGPMFatom(  std::ifstream &file, unsigned long len )
{
    // Go Pro MetaData Format
    //
    //  GPMF uses a Key-Length-Value (KLV) format, where data is organized hierarchically:
    //
    //  Key (4 bytes): FourCC identifier (e.g., DEVC, DVID, DVNM).
    //
    //  Type (1 byte): Data type (e.g., c=char, L=int32, f=float, 0=nested container).
    //  
    //  Size (1 byte): Bytes per element (e.g., 1 for char, 4 for int32).
    //
    //  Repeat (2 bytes): Number of elements (total data length = Size × Repeat).
    //
    //  Data (variable): The values or nested KLV structures.
    
    if( len == 0 )
    {
        std::cout << "[0 bytes]" << std::endl;
    } else {
        // display the entire len
        std::cout << std::dec << len << " bytes : GoPro MetaData Format" << std::endl;

        getKLVData( file, len );

    }
}

void getKLVData( std::ifstream &file, unsigned long len )
{
    m_depth++;
    std::string padding = calcPadding( m_depth );

    while( len > 0 )
    {
        // walk the data structure
        unsigned char key[4];
        file.read( (char *)key, 4 );

        // add special case
        if( key[0] == 0 && key[1] == 0 && key[2] == 0 && key[3] == 0 )
        {
            std::cout << padding << "[\033[1;32m...end...\033[0m]" << std::endl;
            len -= 4;
            // read the rest and discard
            char * buffer;
            buffer = new char[len];
            file.read( buffer, len );
            len = 0;
            delete [] buffer;
            break;
        }

        std::cout << padding << "[\033[1;32m" << key[0] << key[1] << key[2] << key[3] << "\033[0m]";
        len -= 4;

        unsigned char type;
        file.read( (char *)&type, 1 );
        std::string type_str = "?????";
        if( type == 'c' ) type_str = "char  ";
        else if( type == 'L' ) type_str = "int32 ";
        else if( type == 'l' ) type_str = "uint32";
        else if( type == 'F' ) type_str = "float ";
        else if( type == 'B' ) type_str = "byte  ";
        else if( type == 0 ) type_str =   "nest  ";
        else type_str[2] = type;
        std::cout << " (" << type_str;
        len--;

        unsigned char size;
        file.read( (char *)&size, 1 );
        len--;

        unsigned short repeat;
        file.read( (char *)&repeat, 2 );
        len-=2;

        unsigned int count = swapShort(repeat);
        unsigned int total = size * count;

        std::cout << ") " << total << " bytes ";

        // do the right thing
        if( type == 'c' )
        {
            // pad the string out to a 4 byte boundary
            int dif = total % 4;
            if( dif != 0 ) total += 4 - dif;

            std::cout << "<pad to " << total << "> ";

            // char
            char * buffer;
            buffer = new char[total];
            file.read( buffer, total );
            len -= total;

            std::cout << "[";
            for( int i = 0; i < total; i++ ) 
            { 
                // make sure the char is printable
                if( buffer[i] < 32 || buffer[i] > 126 ) std::cout << ".";
                else std::cout << buffer[i]; 
            }
            std::cout << "]" << std::endl;
            delete [] buffer;

        } else if( type == 'L' )
        {
            // unsigned int 32
            unsigned int * buffer;
            buffer = new unsigned int[count];
            file.read( (char *)buffer, total );
            len -= total;

            std::cout << ", " << count << " item(s) [ ";

            for( int i = 0; i < count; i++ ) { std::cout << swapOrder(buffer[i]) << ", "; }
            std::cout << "]" << std::dec << std::endl;
            delete [] buffer;

        } else if( type == 'l' )
        {
            // signed int 32
            int * buffer;
            buffer = new int[count];
            file.read( (char *)buffer, total );
            len -= total;

            std::cout << ", " << count << " item(s) [ ";

            for( int i = 0; i < count; i++ ) { std::cout << (int)swapOrder(buffer[i]) << ", "; }
            std::cout << "]" << std::dec << std::endl;
            delete [] buffer;

        } else if( type == 'F' )
        {
            // float
            float * buffer;
            buffer = new float[count];
            file.read( (char *)buffer, total );
            len -= total;

            std::cout << " : " << count << " items [ ";

            for( int i = 0; i < count; i++ ) { std::cout << swapOrder(buffer[i]) << ", "; }
            std::cout << "]" << std::endl;
            delete [] buffer;

        } else if( type == 0 )
        {
            // nested container
            std::cout << std::endl;

            getKLVData( file, total );
            len -= total;

        } else if( type == 'B' )
        {
            // pad the data out to a 4 byte boundary
            int dif = total % 4;
            if( dif != 0 ) total += 4 - dif;

            // signed int
            char * buffer;
            buffer = new char[total];
            file.read( buffer, total );
            len -= total;

            std::cout << "[ ";
            for( int i = 0; i < total; i++ ) 
            { 
                std::cout << std::dec << (int)buffer[i] << " "; 
            }
            std::cout << "]" << std::endl;
            delete [] buffer;
        }
    }

    m_depth--;
}