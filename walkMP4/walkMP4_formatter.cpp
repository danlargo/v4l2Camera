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


int printVER8data( std::ifstream &file, std::string descrip )
{
    // read only 1 byte
    char val;
    file.read( (char *)&val, 1 );

    std::cout << " " << toLower(descrip) << " <" << (int)val << ">";

    return (int)val;
}


void printFLAGSdata( std::ifstream &file, std::string descrip, int num)
{
    // read num bytes
    char val[num];
    file.read( (char *)&val, num );

    std::cout << " " << toLower(descrip) << " [";
    for( int i = 0; i < num; i++ )
    {
        std::cout << std::hex << (int)val[i] << std::dec << " ";
    }
    std::cout << "]";
}


void printINT8data( std::ifstream &file, std::string descrip )
{
    // read only 4 bytes
    char val;
    file.read( (char *)&val, 1 );

    std::cout << " " << toLower(descrip) << " <" << (int)val << ">";
}

void printUINT8data( std::ifstream &file, std::string descrip )
{
    // read only 4 bytes
    unsigned char val;
    file.read( (char *)&val, 1 );
    val = swapEndian(val);

    std::cout.imbue(std::locale(""));
    std::cout << " " << toLower(descrip) << " <" << val << ">";
    std::locale::classic();
}

void printINT16data( std::ifstream &file, std::string descrip )
{
    // read only 4 bytes
    short val;
    file.read( (char *)&val, 2 );
    val = swapEndian(val);

    std::cout << " " << toLower(descrip) << " <" << (int)val << ">";
}

void printUINT16data( std::ifstream &file, std::string descrip )
{
    // read only 4 bytes
    unsigned short val;
    file.read( (char *)&val, 1 );
    val = swapEndian(val);

    std::cout.imbue(std::locale(""));
    std::cout << " " << toLower(descrip) << " <" << val << ">";
    std::locale::classic();
}


void printINT32data( std::ifstream &file, std::string descrip )
{
    // read only 4 bytes
    int val;
    file.read( (char *)&val, 4 );
    val = swapEndian(val);

    std::cout << " " << toLower(descrip) << " <" << val << ">";
}

void printUINT32data( std::ifstream &file, std::string descrip )
{
    // read only 4 bytes
    unsigned int val;
    file.read( (char *)&val, 4 );
    val = swapEndian(val);

    std::cout.imbue(std::locale(""));
    std::cout << " " << toLower(descrip) << " <" << val << ">";
    std::locale::classic();
}

int printMP4TIMEdata( std::ifstream &file, std::string descrip, int ver )
{
    int ret = 4;

    unsigned int s_time;
    unsigned long l_time;

    if( ver == 0 )
    {
        file.read( (char *)&s_time, 4 );
        s_time = swapEndian(s_time);

        #define MP4_EPOCH_OFFSET 2082844800ULL  // 66 years, accounting for leap years

        time_t tmp_time = (time_t)(s_time - MP4_EPOCH_OFFSET);

        struct tm *time_info;
        char buffer[80];
        if( s_time == 0 ) std::cout << descrip << " < - ";
        else 
        {
            time_info = gmtime(&tmp_time);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
            std::cout << descrip << " <\033[1;33m" << buffer << "\033[0m";
        }

    } else
    {
        file.read( (char *)&l_time, 8 );
        l_time = swapEndian(l_time);

        time_t tmp_time = (time_t)l_time;
        struct tm *time_info = gmtime(&tmp_time);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
        std::cout << descrip << "<\033[1;33m" << buffer << "\033[0m";

        ret = 8;
    }

    return ret;
}

void printHEX32data( std::ifstream &file, std::string descrip )
{
    // read only 4 bytes
    unsigned int val;
    file.read( (char *)&val, 4 );
    val = swapEndian(val);

    std::cout.imbue(std::locale(""));
    std::cout << " " << toLower(descrip) << " <0x" << std::hex << val << std::dec << ">";
    std::locale::classic();
}


void printTAG4data( std::ifstream &file, std::string descrip )
{
    // read only 4 bytes
    unsigned char val[4];
    file.read( (char *)&val, 4 );

    std::cout << " " << toLower(descrip) << " [\033[1;36m" << val[0] << val[1] << val[2] << val[3] << "\033[0m]";
}

void printTAG4LISTdata( std::ifstream &file, std::string descrip, int remaining )
{
    std::cout << " " << toLower(descrip) << " [ \033[1;36m";

    // read only 4 byte tags until no more data
    while( remaining > 0 )
    {
        unsigned char val[4];
        file.read( (char *)&val, 4 );

        std::cout << val[0] << val[1] << val[2] << val[3] << " ";

        remaining -= 4;
    }
    std::cout << "\033[0m]";
}

void printREMAININGdata( std::ifstream &file, int remaining )
{
    // read the rest of the data
    char * buffer = new char[remaining];
    file.read( buffer, remaining );

    std::cout << "  (remaining " << remaining << " bytes) [";
    for( int i = 0; i < remaining; i++ )
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i] << " ";
    } 
    std::cout << "]";
}


void printINT1616data( std::ifstream &file, std::string descrip )
{
    // read the value as two shorts that are printed as "val.val"
    unsigned short val[2];
    file.read( (char *)&val, 4 );
    val[0] = swapEndian(val[0]);
    val[1] = swapEndian(val[1]);

    std::cout << " " << toLower(descrip) << " <" << val[0] << "." << val[1] << ">";

}

void printINT88data( std::ifstream &file, std::string descrip )
{
    // read the value as two shorts that are printed as "val.val"
    char val[2];
    file.read( val, 2 );

    std::cout << " " << toLower(descrip) << " <" << (int)val[0] << "." << (int)val[1] << ">";

}


void printRSRVDdata( std::ifstream &file, std::string descrip, int num )
{
    // read COUNT int32 and print
    char val[num];
    file.read( val, num );
}


void printINT32COUNTdata( std::ifstream &file, std::string descrip, int num )
{
    // read COUNT int32 and print
    int val[num];
    file.read( (char *)&val, 4*num );

    std::cout.imbue(std::locale(""));
    std::cout << " " << toLower(descrip) << " [";
    for( int i = 0; i < num; i++ )
    {
        std::cout << swapEndian(val[i]) << " ";
    }
    std::cout << "]";
    std::locale::classic();
}

void printUINT32COUNTdata( std::ifstream &file, std::string descrip, int num )
{
    // read COUNT int32 and print
    unsigned int val[num];
    file.read( (char *)&val, 4*num );

    std::cout.imbue(std::locale(""));
    std::cout << " " << toLower(descrip) << " [";
    for( int i = 0; i < num; i++ )
    {
        std::cout << swapEndian(val[i]) << " ";
    }
    std::cout << "]";
    std::locale::classic();
}



std::string calcPadding()
{
    std::string ret = "";
    for( int i = 1; i < m_depth; i++ ) ret += "     ";

    return ret;
}

void printDictionary( std::map<std::string, struct node_t*> dictionary )
{
    std::map<std::string, struct node_t*>::iterator it;

    std::cout << std::endl  << "walkMP4 Data Dictionary" << std::endl;
    std::cout               << "=======================" << std::endl;

    for( it = dictionary.begin(); it != dictionary.end(); it++ )
    {
        std::cout << it->first << " : " << it->second->type << " : " << it->second->description << std::endl;
    }

    std::cout << std::endl << "...done print" << std::endl;
}