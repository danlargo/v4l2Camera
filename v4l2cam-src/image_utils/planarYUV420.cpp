#include "image_utils.h"

// Planar (Non-Interleaved) YUV 420 format definition
//
// Y - Luminence, one byte per pixel in first section of file
//
//      00 01 02 03 04 05 06 07                etc
//      08 09 10 11 12 13 14 15
//      ...
//
// U (Cr) - Chroma, one byte per 4 pixels
//
//      00 01 02 03 04 05 06 07                 etc
//      ..
//
//      with mapping of Y =
//      00 01
//      08 09       ==> mapping to 00 of U (Cr)
//
// V (Cb) = Chroma, one byte per 4 pixels
//
//      00 01 02 03 04 05 06 07                 etc
//      ..
//
//      with mapping of Y =
//      00 01
//      08 09       ==> mapping to 00 of V (Cb)
//
//
// File size is (width*height = size)
// Y - size bytes, followed by
// U - size/4 bytes, followed by
// V - size/4 bytes
//
//
unsigned char * planarYUV420ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale )
{
    // output buffer
    unsigned char* rgb_image = new unsigned char[width * height * 3];

    // width and height of the image to be converted
    int size = width * height;
    int sizeU = size/4;

    int outIndex = 0;

    int Y, U, V;
    double R, G, B;

    for( int ii = 0; ii < size; ii++ )
    {
        // get mappings from    00 01       (width = 8, size = width*height)
        //                      08 09
        //                                  to U = size + 00
        //                                  and V = size + (sizeU) + 00
        //
        // offsetU = size + (ii%width) / 2
        // offsetV = sizeU + (ii%width) / 2
        //

        Y = yuv_image[ii];
        int offsetUV = (ii%width) / 2;

        U = yuv_image[size + offsetUV];
        V = yuv_image[size + sizeU + offsetUV];

        R = R_fromYUV( Y, U, V );
        G = R;
        B = R;

        if( !grayScale )
        {
            G = G_fromYUV( Y, U, V );
            B = B_fromYUV( Y, U, V );
        }

        rgb_image[outIndex++] = R;
        rgb_image[outIndex++] = G;
        rgb_image[outIndex++] = B;

    }

    return rgb_image;
}

// same as bove but U and V samples are reversed
//

unsigned char * planarYVU420ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale )
{
    // output buffer
    unsigned char* rgb_image = new unsigned char[width * height * 3];

    // width and height of the image to be converted
    int size = width * height;
    int sizeUV = size/4;

    int outIndex = 0;

    int Y, U, V;
    double R, G, B;

    for( int ii = 0; ii < size; ii++ )
    {
        // get mappings from    00 01       (width = 8, size = width*height)
        //                      08 09
        //                                  to V = size + 00
        //                                  and U = size + (sizeU) + 00
        //
        // offsetV = size + (ii%width) / 2
        // offsetU = sizeV + (ii%width) / 2
        //

        Y = yuv_image[ii];
        int offsetUV = (ii%width) / 2;

        V = yuv_image[size + offsetUV];
        U = yuv_image[size + sizeUV + offsetUV];

        R = R_fromYUV( Y, U, V );
        G = R;
        B = R;

        if( !grayScale )
        {
            G = G_fromYUV( Y, U, V );
            B = B_fromYUV( Y, U, V );
        }

        rgb_image[outIndex++] = R;
        rgb_image[outIndex++] = G;
        rgb_image[outIndex++] = B;

    }

    return rgb_image;
}