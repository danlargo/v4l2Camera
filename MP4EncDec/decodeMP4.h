bool parseFTYPatom( std::ifstream &file, int len, std::string & variant );
void parseMOOVatom( std::ifstream &file, int len );

void parseMVHDatom( std::ifstream &file, int len );

void parseIODSatom( std::ifstream &file, int len );

void parseTRAKatom( std::ifstream &file, int len );
void parseTKHDatom( std::ifstream &file, int len );

void parseMDIAatom( std::ifstream &file, int len );
void parseMDHDatom( std::ifstream &file, int len );
void parseHDLRatom( std::ifstream &file, int len );
void parseMINFatom( std::ifstream &file, int len );

bool checkKnownVariant( std::string name, int ver );

void parseTIMEhdr( std::ifstream &file, unsigned char version, std::string padding );

std::string decode_lang(uint16_t lang);

struct known_variant_t
{
    std::string name;
    int ver;
};

struct known_variant_t known_mp4[3];
int num_known_mp4;

unsigned int m_timescale;

std::string getVersionString();
unsigned int swapOrder( unsigned int in );
unsigned short swapShort( unsigned short in );

#pragma pack(push, 1)
struct atom_t
{
    unsigned int size;
    unsigned char type[4];
};

struct atom_time0_t
{
    unsigned int created;
    unsigned int modified;
} timev0;

struct atom_time1_t
{
    unsigned long created;
    unsigned long modified;
} timev1;

// Version (1 byte): Specifies the version of the MVHD atom format. It’s typically 0x00 (version 0) or 0x01 (version 1).
// Flags (3 bytes): Reserved and usually set to 0x000000. These are not currently used but are included for alignment and future extensions.
struct  version_t
{
    unsigned char version;
    unsigned char flags[3];
} ver;

#pragma pack(pop)