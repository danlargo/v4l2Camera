#include "image_utils.h"

#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <cstring>


bool saveRGB24AsBMP( unsigned char * rgbData, int width, int height, std::string fid )
{
    bool streamToStdout = (fid.length() == 0 );

    typedef struct                       /**** BMP file header structure ****/
    {
        unsigned int   bfSize;           /* Size of file */
        unsigned short bfReserved1;      /* Reserved */
        unsigned short bfReserved2;      /* ... */
        unsigned int   bfOffBits;        /* Offset to bitmap data */
    } bmpFileHeader;

    typedef struct                       /**** BMP file info structure ****/
    {
        unsigned int   biSize;           /* Size of info header */
        int            biWidth;          /* Width of image */
        int            biHeight;         /* Height of image */
        unsigned short biPlanes;         /* Number of color planes */
        unsigned short biBitCount;       /* Number of bits per pixel */
        unsigned int   biCompression;    /* Type of compression to use */
        unsigned int   biSizeImage;      /* Size of image data */
        int            biXPelsPerMeter;  /* X pixels per meter */
        int            biYPelsPerMeter;  /* Y pixels per meter */
        unsigned int   biClrUsed;        /* Number of colors used */
        unsigned int   biClrImportant;   /* Number of important colors */
    } bmpInfoHeader;

    std::ofstream pFile;
    if( !streamToStdout )
    {
        pFile.open(fid, std::ios::trunc | std::ios::binary );
        if( !pFile.is_open() )
        {
            #ifdef _WIN32
            std::array<char, 256> errorBuffer;
            strerror_s(errorBuffer.data(), errorBuffer.size(), errno);
            std::cerr << "Failed to open output file : " << fid << " - " + std::string(errorBuffer.data()) << std::endl;
    #else
            std::cerr << "[\x1b[1;31mwarning\x1b[0m] Failed to open file for writing : " << strerror(errno) << std::endl;
    #endif
            return false;
        }
    }

    bmpFileHeader bfh;
    bmpInfoHeader bih;

    /* Magic number for file. It does not fit in the header structure due to alignment requirements, so put it outside */
    unsigned short bfType = 0x4d42;
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfSize = 2 + sizeof(bmpFileHeader) + sizeof(bmpInfoHeader) + width * height * 3;
    bfh.bfOffBits = 0x36;

    bih.biSize = sizeof(bmpInfoHeader);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = 0;
    bih.biSizeImage = 0;
    bih.biXPelsPerMeter = 5000;
    bih.biYPelsPerMeter = 5000;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    /* Write headers */
    if( streamToStdout )
    {
        std::cout.write((const char *) &bfType, sizeof(bfType));
        std::cout.write((const char *) &bfh, sizeof(bfh));
        std::cout.write((const char *) &bih, sizeof(bih));
        // now write the bitmap
        std::cout.write((const char *)rgbData, width* height * 3);

        // fluch the stream
        std::cout.flush();

        std::cerr << "[\x1b[1;33minfo\x1b[0m] Image output to STDOUT" << std::endl;

    } else {
        pFile.write( (char *)&bfType, sizeof(bfType) );
        pFile.write( (char *)& bfh, sizeof(bfh) );
        pFile.write( (char *)& bih, sizeof(bih) );

        /* Write bitmap */
        pFile.write((char *)rgbData, width* height * 3);

        pFile.close();

        std::cerr << "[\x1b[1;33minfo\x1b[0m] Image saved to : " << fid << std::endl;
    }

    

    return true;
}