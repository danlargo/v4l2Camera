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


struct mp4StreamInfo_t
{
    unsigned int frame_count;
    unsigned int width;
    unsigned int height;
    unsigned int frame_rate;
    unsigned char * sps;
    unsigned char * pps;
    unsigned short sps_size;
    unsigned short pps_size;
    std::vector<unsigned int> frame_sizes;
    unsigned int timeScale;
    unsigned int timeScale_swapped;
    unsigned int videoDuration;
    unsigned int ticks_per_frame;
    unsigned int mdatSize;
};

// Version Utils
//
extern const int s_majorVersion;
extern const int s_minorVersion;
extern const int s_revision;

struct h264FrameHeader_t * getFrameHeader( std::ifstream & inFile );
struct mp4StreamInfo_t * getStreamInfo( std::ifstream & inFile );

bool writeMP4Header( std::fstream & file, struct mp4StreamInfo_t * info );
bool writeFTYPatom( std::fstream & file );
bool writeFREEatom( std::fstream & file );
bool writeMOOVatom( std::fstream & file );
int writeAUTHatom( std::fstream & file );
int writeMVHDatom( std::fstream & file );
int writeTRAKatom( std::fstream & file );
int writeTKHDatom( std::fstream & file );

int writeMDIAatom( std::fstream & file );
int writeMDHDatom( std::fstream &file );
int writeHDLRatom( std::fstream &file );
int writeMINFatom( std::fstream &file );

int writeVMHDatom( std::fstream &file );
int writeDINFatom( std::fstream &file );

int writeSTBLatom( std::fstream &file );
int writeSTSDatom( std::fstream &file );
int writeSTTSatom( std::fstream &file );
int writeSTSCatom( std::fstream &file );
int writeSTSZatom( std::fstream &file );
int writeSTCOatom( std::fstream &file );

bool writeMDATatom( std::ifstream &file, std::fstream & outfile );
bool verifyMP4Data( std::fstream & file, struct mp4StreamInfo_t * info, int mdatStartLocation );

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