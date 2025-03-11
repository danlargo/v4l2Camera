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
    
    if( !std::isprint(atom.tag[0] ) ) 
    {
        std::cout << "[\033[1;32m" << std::hex << std::setw(2) << std::setfill('0') 
        << (unsigned int)(unsigned char)atom.tag[0] << " " << (unsigned int)(unsigned char)atom.tag[1] << " "
        << (unsigned int)(unsigned char)atom.tag[2] << " " << (unsigned int)(unsigned char)atom.tag[3] << " "
        << std::dec << "\033[0m] \033[1;37m" << descrip << "\033[0m" << std::endl;
    } else std::cout << "[\033[1;32m" << atom.tag << "\033[0m]";

    if( descrip.length() > 0 )  std::cout << " \033[1;37m" << descrip << "\033[0m" << std::endl;
    else 
    {
        std::cout.imbue(std::locale(""));
        std::cout << std::dec << "\033[1;33m(" << atom.size << " bytes)  ...not in dictionary\033[0m" << std::endl;
        std::locale::classic();
    }

}


void printFREEdata( std::ifstream &file, int size )
{
    m_depth++;

    unsigned int dump_len = 64;
    if( size < dump_len ) dump_len = size;

    // we can grab this all at once as it is not likely to be fucking huge
    char * buffer = new char[size];
    file.read( buffer, size );

    // dump up to 80 chars that we find in the data
    std::cout << std::dec << calcPadding();

    std::cout << "  (" << size << " bytes) ";

    if( size > 0 )
    {
        // now dump the raw bytes
        for( int i = 0; i < (dump_len/2); i++ )
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i] << " ";
        }

        // dump any printable characters
        std::cout << std::endl << std::dec << calcPadding() << "  [\033[38;5;12m";
        for( int i = 0; i < dump_len; i++ )
        {
            if( isprint(buffer[i]) ) std::cout << buffer[i];
        }
        std::cout << "\033[0m]";
    }

    delete [] buffer;

    std::cout << std::endl;

    m_depth--;
}

void printRAWdata( std::ifstream &file, int size )
{
    m_depth++;

    int chunk = 2048;
    int grab_size = size;

    std::cout.imbue(std::locale(""));
    std::cout << calcPadding() << std::dec << "  (binary data) " << size << " bytes" << std::endl;
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


void printCHARSdata( std::ifstream &file, int size)
{
    m_depth++;

    std::cout.imbue(std::locale(""));
    std::cout << calcPadding() << std::dec << "  (chars) " << size << " bytes";
    std::locale::classic();

    char * buffer = new char[size+1];
    file.read( buffer, size );
    buffer[size] = 0;

    std::cout << " : \033[0;36m" << buffer << "\033[0m" << std::endl;

    delete [] buffer;

    m_depth--;
}

void printUNKNdata( std::ifstream &file, int size )
{
    m_depth++;
    int chunk = 16;
    int num_lines = 2;

    int grab_size = size;

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


int printVER8data( std::ifstream &file, struct node_t * n )
{
    // read only 1 byte
    char val;
    file.read( (char *)&val, 1 );

    std::string units = "";
    if( n->units.length() > 0 ) units = " " + toLower(n->units);

    std::cout << " " << toLower(n->description) << " <" << (int)val << units << ">";

    return (int)val;
}


void printFLAGSdata( std::ifstream &file, struct node_t * n )
{
    // read num bytes
    char val[n->count];
    file.read( (char *)&val, n->count );

    std::cout << " " << toLower(n->description) << " [";
    for( int i = 0; i < n->count; i++ )
    {
        std::cout << std::hex << (int)val[i] << std::dec;
        if( i < n->count-1 ) std::cout << " ";
    }
    std::cout << "]";
}

void printINTEGERdata( std::ifstream &file, struct node_t * n, bool unSigned, bool hex, int width )
{
    // declare all the data items e might need
    int val_int[n->count];
    unsigned int val_uint[n->count];
    char val_char[n->count];
    unsigned char val_uchar[n->count];
    short val_short[n->count];
    unsigned short val_ushort[n->count];

    int num_to_read = width * n->count;

    if( unSigned )
    {
        if( width == 1 ) file.read( (char *)&val_uchar, num_to_read );
        else if( width == 2 ) file.read( (char *)&val_ushort, num_to_read );
        else file.read( (char *)&val_uint, num_to_read );

    } else
    {
        if( width == 1 ) file.read( (char *)&val_char, num_to_read );
        else if( width == 2 ) file.read( (char *)&val_short, num_to_read );
        else file.read( (char *)&val_int, num_to_read );
    }

    std::cout << " " << toLower(n->description) << " [";

    // add commas but only if it is not hex
    if( !hex ) std::cout.imbue(std::locale(""));
    else std::cout.imbue(std::locale::classic());

    for( int i = 0; i < n->count; i++ )
    {
        if( hex ) std::cout << std::hex << "\033[0;35m0x";
        else std::cout << std::dec << "\033[1;33m";

        if( unSigned )
        {
            if( width == 1 ) 
            {
                std::cout << (int)val_uchar[i];
                // check if we need to assign this to a variable
                if( n->var1.length() > 0 ) m_vars[n->var1] = std::to_string((int)val_uchar[i]);
            }
            else if( width == 2 ) 
            {
                std::cout << swapEndian(val_ushort[i]);
                // check if we need to assign this to a variable
                if( n->var1.length() > 0 ) m_vars[n->var1] = std::to_string(swapEndian(val_ushort[i]));
            }
            else 
            {
                std::cout << swapEndian(val_uint[i]);
                // check if we need to assign this to a variable
                if( n->var1.length() > 0 ) m_vars[n->var1] = std::to_string(swapEndian(val_uint[i]));
            }
        } else
        {
            if( width == 1 ) 
            {
                std::cout << (int)val_char[i];
                // check if we need to assign this to a variable
                if( n->var1.length() > 0 ) m_vars[n->var1] = std::to_string((int)val_char[i]);
            }
            else if( width == 2 ) 
            {
                std::cout << swapEndian(val_short[i]);
                // check if we need to assign this to a variable
                if( n->var1.length() > 0 ) m_vars[n->var1] = std::to_string(swapEndian(val_short[i]));
            }
            else 
            {
                std::cout << swapEndian(val_int[i]);
                // check if we need to assign this to a variable
                if( n->var1.length() > 0 ) m_vars[n->var1] = std::to_string(swapEndian(val_int[i]));
            }
        }

        if( i < n->count-1 ) std::cout << ", ";
    }

    std::string units = "";
    if( n->units.length() > 0 ) units = " " + toLower(n->units);

    std::cout << std::dec << units << "\033[0m]";
    std::cout.imbue(std::locale::classic());
}


int printMP4TIMEdata( std::ifstream &file, struct node_t * n, int ver )
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
        if( s_time == 0 ) std::cout << " " << toLower(n->description) << " [ - ]";
        else 
        {
            time_info = gmtime(&tmp_time);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
            std::cout << " " << toLower(n->description) << " [\033[38;5;1m" << buffer << "\033[0m]";
        }

    } else
    {
        file.read( (char *)&l_time, 8 );
        l_time = swapEndian(l_time);

        time_t tmp_time = (time_t)l_time;
        struct tm *time_info = gmtime(&tmp_time);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", time_info);
        std::cout << " " << toLower(n->description) << " [\033[38;5;1m" << buffer << "\033[0m]";

        ret = 8;
    }

    return ret;
}


int printMP4TICKSdata( std::ifstream &file, struct node_t * n, int ver )
{
    int ret = 4;

    unsigned int s_dur;
    unsigned long l_dur;

    std::cout.imbue(std::locale(""));

    if( ver == 0 )
    {
        file.read( (char *)&s_dur, 4 );
        s_dur = swapEndian(s_dur);

        std::cout << " " << toLower(n->description) << " [\033[1;33m" << s_dur;

        // check if we need to assign this to a variable
        if( n->var1.length() > 0 ) m_vars[n->var1] = std::to_string(s_dur);

    } else
    {
        file.read( (char *)&l_dur, 8 );
        l_dur = swapEndian(l_dur);

        std::cout << " " << toLower(n->description) << " [\033[1;33m" << l_dur ;

        // check if we need to assign this to a variable
        if( n->var1.length() > 0 ) m_vars[n->var1] = std::to_string(s_dur);

        ret = 8;
    }

    std::string units = "";
    if( n->units.length() > 0 ) units = " " + toLower(n->units);
    std::cout << units << "\033[0m]";

    std::cout.imbue(std::locale::classic());

    return ret;
}

// Grab one or more TAG4 fields
//
void printTAG4data( std::ifstream &file, struct node_t * n )
{
    // read only 4 bytes
    unsigned char val[4];
    file.read( (char *)&val, 4 );

    std::string units = "";
    if( n->units.length() > 0 ) units = " " + toLower(n->units);

    std::cout << " " << toLower(n->description) << " [\033[1;36m" << val[0] << val[1] << val[2] << val[3] << units << "\033[0m]";
}

void printLANGdata( std::ifstream &file, struct node_t * n )
{
    // read only 2 bytes
    unsigned short val;
    file.read( (char *)&val, 2 );

    std::cout << " " << toLower(n->description) << " <\033[1;36m" << decodeLang(swapEndian(val)) << "\033[0m>";
}

std::string decodeLang(unsigned short lang) 
{
    // Extract 5-bit values
    unsigned char char1 = (lang >> 10) & 0x1F;  // Bits 14–10
    unsigned char char2 = (lang >> 5) & 0x1F;   // Bits 9–5
    unsigned char char3 = lang & 0x1F;          // Bits 4–0

    // Convert to ASCII by adding 0x60
    std::string result;

    result += (char1 ? char1 + 0x60 : ' ');
    result += (char2 ? char2 + 0x60 : ' ');
    result += (char3 ? char3 + 0x60 : ' ');

    trim(result);

    result += lookupLang(result);

    return result;
}

std::string lookupLang( std::string short_lang )
{
    std::string ret = " : undefined";

    if( short_lang == "eng" ) ret = " : english";
    else if( short_lang == "" ) ret = "not set";
    else if( short_lang == "fra" ) ret = " : french";
    else if( short_lang == "deu" ) ret = " : german";
    else if( short_lang == "ita" ) ret = " : italian";
    else if( short_lang == "spa" ) ret = " : spanish";
    else if( short_lang == "dut" ) ret = " : dutch";

    return ret;
}


// Grab tags until the remaining bytes are consumed
//
void printTAG4LISTdata( std::ifstream &file, struct node_t * n, int remaining )
{
    std::cout << " " << toLower(n->description) << " [ \033[1;36m";

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

// Grab a null terminated string
//
int printSTRINGdata( std::ifstream &file, struct node_t * n, int max_len )
{
    int ret = 0;
    char val;

    std::string name = "";
    
    // read and output a character at time until we hit a null
    std::cout << " " << toLower(n->description) << " <\033[1;36m";

    while( true )
    {
        file.read( &val, 1 );
        if( val == 0 ) break;

        name += val;
        ret++;

        // check if we have gone too long
        if( ret >= max_len ) break;
    }

    trim(name);
    std::cout << name;

    std::string units = "";
    if( n->units.length() > 0 ) units = " " + toLower(n->units);    
    std::cout << "\033[0m>" << units;

    return ret;
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


void printINT1616data( std::ifstream &file, struct node_t * n, bool unSigned )
{
    // read the value as two shorts that are printed as "val.val"
    short val[2*n->count];
    unsigned short val2[2*n->count];

    if( unSigned ) file.read( (char *)&val2, 4*n->count );
    else file.read( (char *)&val, 4*n->count );

    std::cout << " " << toLower(n->description) << " <\033[1;33m";

    for( int i=0; i<n->count; i++ )
    {
        if( unSigned ) std::cout << swapEndian(val2[0]) << "." << swapEndian(val2[1]);
        else std::cout << swapEndian(val[0]) << "." << swapEndian(val[1]);

        if( i < n->count-1 ) std::cout << ", ";
    }

    std::string units = "";
    if( n->units.length() > 0 ) units = " " + toLower(n->units);
    std::cout << units << "\033[0m>";
}

void printINT88data( std::ifstream &file, struct node_t * n, bool unSigned )
{
    // read the value as two shorts that are printed as "val.val"
    char val[2*n->count];
    unsigned char val2[2*n->count];

    if( unSigned ) file.read( (char *)&val2, 2*n->count );
    else file.read( val, 2*n->count );

    std::string units = "";
    if( n->units.length() > 0 ) units = " " + toLower(n->units);

    std::cout << " " << toLower(n->description) << " <\033[1;33m";

    for( int i=0; i<n->count; i++ )
    {
        if( unSigned ) std::cout << (unsigned int)(unsigned char)val[0] << "." << (unsigned int)(unsigned char)val[1];
        else std::cout << (int)val[0] << "." << (int)val[1];

        if( i < n->count-1 ) std::cout << ", ";
    }

    std::cout << units << "\033[0m>";

}


void skipIGNOREdata( std::ifstream &file, int num )
{
    // read data and just ignore it
    char val[num];
    file.read( val, num );
}

void printFORMAT( struct node_t * n )
{
    if( n->type == "NEWLINE" ) std::cout << std::endl << calcPadding() << " ";
    else if( n->type == "LABEL" ) std::cout << " " << toLower(n->description);
}

void printMATHdata( struct node_t * n )
{
    // check what type we need to do
    if( n->type == "MATH_DIV" )
    {
        // get the variables
        std::string var1 = m_vars[n->var1];
        std::string var2 = m_vars[n->var2];

        // do the operation
        std::string units = "";
        if( n->units.length() > 0 ) units = " " + toLower(n->units);

        // fix up the description, in case it is blank
        std::string desc = "";
        if( n->description.length() > 0 ) desc = " " + toLower(n->description);

        try
        {
            double result = std::stod(var1) / std::stod(var2);
            std::cout << desc << " [\033[1;33m" << std::fixed << std::setprecision(2) << result << units << "\033[0m]";
        }
        catch(const std::exception& e)
        {
            std::cout << desc << " MathDiv operation failed : " << e.what();
        }
        

    } else std::cout << "unknown math operation requested : " << n->type;
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