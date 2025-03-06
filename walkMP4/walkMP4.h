#include <map>
#include <string>

struct atom_t
{
    unsigned char orig[5];
    std::string tag;
    unsigned long size;
    unsigned bytes_read;
};

struct node_t
{
    std::string name;
    std::string type;
    std::string description;
    std::map<std::string, struct node_t> parts;
    std::string raw_data;
    std::string raw_parts;
};

std::map<std::string, struct node_t> parseDictionary( std::string fid );
void printDictionary( std::map<std::string, struct node_t> dictionary );
struct node_t getNode( std::string buf, int &offset );
std::string getRawData( std::string buf, int &offset );
std::string getVal( std::string buf, std::string key );

// parsers
//
void parseATOM( std::ifstream &file, std::map<std::string, struct node_t> dictionary, int size );
void printATOMhdr( struct atom_t atom, std::string descrip );
struct atom_t readATOM( std::ifstream &file );
void printFREEatom( std::ifstream &file, struct atom_t atom );
void printRAWatom( std::ifstream &file, struct atom_t atom );
void printCHARSatom( std::ifstream &file, struct atom_t atom );
void printUNKNatom( std::ifstream &file, struct atom_t atom );

// Outout Formatting
//
extern unsigned int m_depth;
std::string calcPadding();


// Version Utils
//
extern const int s_majorVersion;
extern const int s_minorVersion;
extern const int s_revision;

std::string getVersionString();

// Endian Swapping
//
unsigned int swapEndian( unsigned int in );
unsigned short swapEndian( unsigned short in );
unsigned long swapEndian( unsigned long in );

// String utils
//
std::string toUpper(std::string str);

// Trim from start (in place)
void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);