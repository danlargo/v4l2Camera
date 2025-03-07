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
    int count;
    std::map<std::string, struct node_t*> parts;
    std::string raw_data;
    std::string raw_parts;
};

std::map<std::string, struct node_t*> parseDictionary( std::string fid );
void printDictionary( std::map<std::string, struct node_t*> dictionary );
struct node_t * getNode( std::string buf, int &offset );
std::string getRawData( std::string buf, int &offset );
std::string getVal( std::string buf, std::string key, char start_del, char stop_del );
std::map<std::string, struct node_t*> getParts( std::string buf );

// parsers
//
void parseATOM( std::ifstream &file, std::map<std::string, struct node_t*> dictionary, int size );
void parseSTRUCT( std::ifstream &file, struct node_t * node, struct atom_t atom );

void printATOMhdr( struct atom_t atom, std::string descrip );
struct atom_t readATOM( std::ifstream &file );
void printFREEatom( std::ifstream &file, struct atom_t atom );
void printRAWatom( std::ifstream &file, struct atom_t atom );
void printCHARSatom( std::ifstream &file, struct atom_t atom );
void printUNKNatom( std::ifstream &file, struct atom_t atom );

int printVER8data( std::ifstream &file, std::string descrip );
void printFLAGSdata( std::ifstream &file, std::string descrip, int num );

void printINT8data( std::ifstream &file, std::string descrip );
void printUINT8data( std::ifstream &file, std::string descrip );
void printINT16data( std::ifstream &file, std::string descrip );
void printUINT16data( std::ifstream &file, std::string descrip );
void printINT32data( std::ifstream &file, std::string descrip );
void printUINT32data( std::ifstream &file, std::string descrip );
void printHEX32data( std::ifstream &file, std::string descrip );
void printTAG4data( std::ifstream &file, std::string descrip );

void printREMAININGdata( std::ifstream &file, int remaining );
void printRSRVDdata( std::ifstream &file, std::string descrip, int num );

int printMP4TIMEdata( std::ifstream &file, std::string descrip, int version );

void printINT1616data( std::ifstream &file, std::string descrip );
void printINT88data( std::ifstream &file, std::string descrip );
void printINT32COUNTdata( std::ifstream &file, std::string descrip, int num );
void printUINT32COUNTdata( std::ifstream &file, std::string descrip, int num );
void printTAG4LISTdata( std::ifstream &file, std::string descrip, int remaining );


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
int swapEndian( int in );
unsigned short swapEndian( unsigned short in );
unsigned long swapEndian( unsigned long in );

// String utils
//
std::string toUpper(std::string str);
std::string toLower(std::string str);

// Trim from start (in place)
void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);