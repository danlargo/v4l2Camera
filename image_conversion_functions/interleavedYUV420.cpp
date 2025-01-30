#include "image_utils.h"

// Interleaved YUV 420 format definition
//
// Y - Luminence, one byte per pixel in first section of file
//
//      00 01 02 03 04 05 06 07                etc
//      08 09 10 11 12 13 14 15
//      ...
//
// U, V (Cr, Cb) - Chroma, one alternating bytes per 4 pixels
//
//      U0 V0 U1 V1 U2 V2 U3 V3                 etc
//      ..
//
//      with mapping of Y =
//      00 01
//      08 09       ==> mapping to 00 of U (Cr) and 00 of V (Cb)
//
//      and
//      02 03
//      10 11       ==> mapping to 01 of U (Cr) and 01 of V (Cb)
//
// File size is (width*height = size)
// Y - size bytes, followed by
// U/V - size/2 bytes, followed by
//
//
unsigned char * interleavedYUV420ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale )
{
    // output buffer
    unsigned char* rgb_image = new unsigned char[width * height * 3];

    // width and height of the image to be converted
    int size = width * height;

    int outIndex = 0;

    int Y, U, V;
    double R, G, B;

    for( int ii = 0; ii < size; ii++ )
    {
        // get mappings from    00 01       (width = 8, size = width*height)
        //                      08 09
        //                                  to  U = size + 00
        //                                  and V = size + 01
        //
        // get mappings from    02 03       (width = 8, size = width*height)
        //                      10 11
        //                                  to  U = size + 02
        //                                  and V = size + 03
        //
        // offsetU = size + (ii%width) / 2
        //

        Y = yuv_image[ii];
        int offsetUV = (ii%2)*2;

        U = yuv_image[size + offsetUV];
        V = yuv_image[size + offsetUV + 1];

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


// same algorithm as above, with U and V reversed in the interleaving
//
unsigned char * interleavedYVU420ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale )
{
    // output buffer
    unsigned char* rgb_image = new unsigned char[width * height * 3];

    // width and height of the image to be converted
    int size = width * height;

    int outIndex = 0;

    int Y, U, V;
    double R, G, B;

    for( int ii = 0; ii < size; ii++ )
    {
        // get mappings from    00 01       (width = 8, size = width*height)
        //                      08 09
        //                                  to  U = size + 00
        //                                  and V = size + 01
        //
        // get mappings from    02 03       (width = 8, size = width*height)
        //                      10 11
        //                                  to  U = size + 02
        //                                  and V = size + 03
        //
        // offsetU = size + (ii%width) / 2
        //

        Y = yuv_image[ii];
        int offsetUV = (ii%2)*2;

        V = yuv_image[size + offsetUV];
        U = yuv_image[size + offsetUV + 1];

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