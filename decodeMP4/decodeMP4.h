bool parseFTYPatom( std::ifstream &file, unsigned long len );
void parseMOOVatom( std::ifstream &file, unsigned long len );
void parseUDTAatom( std::ifstream &file, unsigned long len );
void parseMDATatom( std::ifstream &file, unsigned long len );
void parseFREEatom( std::ifstream &file, unsigned long len );
void parseMETAatom( std::ifstream &file, unsigned long len, bool checkVersion );
void parseILSTatom( std::ifstream &file, unsigned long len );
void parseCOPYatom( std::ifstream &file, struct atom_t atom );
void parseQTCOPYatom( std::ifstream &file, struct atom_t atom );
void parseEMPTYatom( std::ifstream &file, struct atom_t atom );
void parseSTRNGatom( std::ifstream &file, unsigned long len );
void parseRAWatom( std::ifstream &file, unsigned long len );
void parseAUTHatom( std::ifstream &file, unsigned long len );
void parseSMTAatom( std::ifstream &file, unsigned long len );
void parseKEYSatom( std::ifstream &file, unsigned long len );
void parseGPMFatom( std::ifstream &file, unsigned long len );

void parseMVHDatom( std::ifstream &file, unsigned long len );

void parseIODSatom( std::ifstream &file, unsigned long len );

void parseTRAKatom( std::ifstream &file, unsigned long len );
void parseTKHDatom( std::ifstream &file, unsigned long len );
void parseTREFatom( std::ifstream &file, unsigned long len );
void parseTAPTatom( std::ifstream &file, unsigned long len );

void parseEDTSatom( std::ifstream &file, unsigned long len );
void parseELSTatom( std::ifstream &file, unsigned long len );

void parseMDIAatom( std::ifstream &file, unsigned long len );

void parseMDHDatom( std::ifstream &file, unsigned long len );
void parseHDLRatom( std::ifstream &file, unsigned long len );
void parseMINFatom( std::ifstream &file, unsigned long len );
void parseDINFatom( std::ifstream &file, unsigned long len );
void parseDREFatom( std::ifstream &file, unsigned long len );
void parseSMHDatom( std::ifstream &file, unsigned long len );
void parseVMHDatom( std::ifstream &file, unsigned long len );
void parseVMHDatom( std::ifstream &file, unsigned long len );
void parseGMHDatom( std::ifstream &file, unsigned long len );
void parseGMINatom( std::ifstream &file, unsigned long len );
void parseTMCDatom( std::ifstream &file, unsigned long len );
void parseTCMIatom( std::ifstream &file, unsigned long len );
void parseSTBLatom( std::ifstream &file, unsigned long len );
void parseSTSDatom( std::ifstream &file, unsigned long len );
void parseSTTSatom( std::ifstream &file, unsigned long len );
void parseSTSCatom( std::ifstream &file, unsigned long len );
void parseUNKNatom( std::ifstream &file, unsigned long len );

bool checkKnownVariant( std::string name, int ver );
void getKLVData( std::ifstream &file, unsigned long len );

void parseH264codec( std::ifstream &file, unsigned int size );
void parseAACcodec( std::ifstream &file, unsigned int size );
void parseCOLRconfig( std::ifstream &file, unsigned int size );
void parseAVCCconfig( std::ifstream &file, unsigned int size );
void parseESDSconfig( std::ifstream &file, unsigned int size );


void parseTIMEhdr( std::ifstream &file, unsigned char version, std::string padding );
struct atom_t getATOMhdr( std::ifstream &file );

std::string decode_lang(uint16_t lang);

std::string getVersionString();
unsigned int swapOrder( unsigned int in );
unsigned short swapShort( unsigned short in );
unsigned long swapLong( unsigned long in );

std::string toUpper(std::string str);

// Trim from start (in place)
void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);

std::string calcPadding( int depth );

extern struct known_variant_t known_mp4[];
extern const int num_known_mp4;
extern unsigned int m_timescale;
extern const int s_majorVersion;
extern const int s_minorVersion;
extern const int s_revision;
extern int m_depth;

#pragma pack(push, 1)
struct atom_t
{
    unsigned char orig[5];
    std::string tag;
    unsigned long size;
    unsigned bytes_read;
};

struct atom_time0_t
{
    unsigned int created;
    unsigned int modified;
};

struct atom_time1_t
{
    unsigned long created;
    unsigned long modified;
};

struct known_variant_t
{
    std::string name;
    int ver;
};

// Version (1 byte): Specifies the version of the MVHD atom format. It’s typically 0x00 (version 0) or 0x01 (version 1).
// Flags (3 bytes): Reserved and usually set to 0x000000. These are not currently used but are included for alignment and future extensions.
struct version_t
{
    unsigned char version;
    unsigned char flags[3];
};

#pragma pack(pop)