#include <map>
#include <vector>
#include <string>

// frame header format from the v4l2camera utility
//
#pragma pack(push, 1)
struct h264FrameHeader_t
{
    unsigned char delimiter[4];
    unsigned int rate;
    unsigned int width;
    unsigned int height;
    unsigned int h264_frame_type;
    unsigned int sps_offset;
    unsigned int pps_offset;
    unsigned int frame_offset;
    unsigned int frame_size;
};
#pragma pack(pop)


// Version Utils
//
extern const int s_majorVersion;
extern const int s_minorVersion;
extern const int s_revision;

struct h264FrameHeader_t * getFrameHeader( std::ifstream & inFile );
bool writeRawData( std::ofstream & file, char * buf, int len );

bool writeMP4Header( std::ofstream & file, int width, int height );
bool writeFTYPatom( std::ofstream & file );
bool writeFREEatom( std::ofstream & file );
bool writeMOOVatom( std::ofstream & file );
int writeAUTHatom( std::ofstream & file );
int writeMVHDatom( std::ofstream & file );
int writeTRAKatom( std::ofstream & file );
int writeTKHDatom( std::ofstream & file );

int writeMDIAatom( std::ofstream & file );
int writeMDHDatom( std::ofstream &file );
int writeHDLRatom( std::ofstream &file );
int writeMINFatom( std::ofstream &file );

int writeVMHDatom( std::ofstream &file );
int writeDINFatom( std::ofstream &file );

int writeSTBLatom( std::ofstream &file );
int writeSTSDatom( std::ofstream &file );
int writeSTTSatom( std::ofstream &file );
int writeSTSCatom( std::ofstream &file );
int writeSTSZatom( std::ofstream &file );
int writeSTCOatom( std::ofstream &file );

bool writeMDATatom( std::ofstream & file );
bool updateAllSizeAndDuration( std::ofstream & file, int frame_rate, int frame_count );

std::string decode_lang(uint16_t lang);
uint16_t encode_lang(std::string lang);

std::string mp4ToString( unsigned int s_time );
unsigned int mp4ToInt32( std::string fmt_str );
unsigned int buildINT16_2( unsigned short a, unsigned short b );


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