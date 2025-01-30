#include "image_utils.h"

// linear 16 bit grey scale values
//
unsigned char * gs16ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale )
{
    unsigned char* rgb_image = new unsigned char[width * height * 3];

    //width and height of the image to be converted

    int outIndex = 0;

    // increment across two bytes for each sample
    //
    for( int ii = 0; ii < width*height*2; ii+=2 )
    {
        // just use the upper byte
        // we could convert and then downscale but you would end up with the upper byte anyway
        //
        // (byte1*256 + byte2 ) / 256 => byte1
        //
        int R = yuv_image[ii];

        rgb_image[outIndex++] = R;
        rgb_image[outIndex++] = R;
        rgb_image[outIndex++] = R;
    }

    return rgb_image;
}


// linear 8-bit grey scale values
//
unsigned char * gs8ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale )
{
    unsigned char* rgb_image = new unsigned char[width * height * 3];

    //width and height of the image to be converted

    int outIndex = 0;

    // increment by 1 byte
    //
    for( int ii = 0; ii < width*height*2; ii++ )
    {
        // single grey scale byte to R and G and B
        //
        int R = yuv_image[ii];

        rgb_image[outIndex++] = R;
        rgb_image[outIndex++] = R;
        rgb_image[outIndex++] = R;
    }

    return rgb_image;
}