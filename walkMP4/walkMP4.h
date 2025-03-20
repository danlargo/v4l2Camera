#include <map>
#include <vector>
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
    std::string units;
    std::string var1;
    std::string var2;
    std::string var3;
    std::string varCount;
    std::string addBreak;
    std::string printResult;
    int count;
    std::vector<struct node_t*> parts;
    std::string raw_data;
    std::string raw_parts;
};

std::map<std::string, struct node_t*> parseDictionary( std::string fid );
void printDictionary( std::map<std::string, struct node_t*> dictionary );
struct node_t * getNode( std::string buf, int &offset );
std::string getNodeData( std::string buf, int &offset );
std::string getNodeName( std::string buf, int &offset );
std::string getVal( std::string buf, std::string key, char start_del, char stop_del );
std::vector<struct node_t*> getParts( std::string buf );
std::string getInner( std::string in );
std::string removeInner( std::string in );

std::string getExecutablePath();

// parsers
//
void parseATOM( std::ifstream &file, std::map<std::string, struct node_t*> dictionary, int size );
void parseSTRUCT( std::ifstream &file, std::map<std::string, struct node_t*> dictionary, struct node_t * node, int size );
void parseLISTdata( std::ifstream &file, std::map<std::string, struct node_t*> dictionary, struct node_t * node, int size );

void printUNKNdata( std::ifstream &file, struct atom_t atom );

void printATOMhdr( struct atom_t atom, std::string descrip );
struct atom_t readATOM( std::ifstream &file );

void printFREEdata( std::ifstream &file, unsigned int size );
void printRAWdata( std::ifstream &file, int size );
void printCHARSdata( std::ifstream &file, int size );
void printBYTEdata( std::ifstream &file, int size );

void printBYTEdata( std::ifstream &file, struct node_t * n );
void printCHARSdata( std::ifstream &file, struct node_t * n );

int printVER8data( std::ifstream &file, struct node_t * n );
void printFLAGSdata( std::ifstream &file, struct node_t * n );

void printINTEGERdata( std::ifstream &file, struct node_t * n, bool unSigned, bool hex, int width );

void printTAG4data( std::ifstream &file, struct node_t * n );
void printTAG4LISTdata( std::ifstream &file, struct node_t * n, int remaining );

int printSTRINGdata( std::ifstream &file, struct node_t * n, int max );
void printLANGdata( std::ifstream &file, struct node_t * n );
std::string decodeLang(unsigned short lang);
std::string lookupLang(std::string lang);

void printREMAININGdata( std::ifstream &file, int remaining );
void skipIGNOREdata( std::ifstream &file, int size );

int printMP4TIMEdata( std::ifstream &file, struct node_t * n, int version );
int printMP4TICKSdata( std::ifstream &file, struct node_t * n, int version );

void printINT1616data( std::ifstream &file, struct node_t * n, bool unSigned );
void printINT88data( std::ifstream &file, struct node_t * n, bool unSigned );

void printFORMAT( struct node_t * n );

void printMATHdata( struct node_t * n );

// Outout Formatting
//
extern unsigned int m_depth;
extern unsigned int m_struct_depth;
std::string calcPadding();


// Version Utils
//
extern const int s_majorVersion;
extern const int s_minorVersion;
extern const int s_revision;

extern std::map<std::string, std::string> m_vars;

extern bool expandLists;

std::string getVersionString();

// Endian Swapping
//
unsigned int swapEndian( unsigned int in );
int swapEndian( int in );
unsigned short swapEndian( unsigned short in );
unsigned long swapEndian( unsigned long in );
short swapEndian( short in );
long swapEndian( long in );

// String utils
//
std::string toUpper(std::string str);
std::string toLower(std::string str);

// Trim from start (in place)
void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);