#include "image_utils.h"

unsigned char * yvu422ToRGB( unsigned char * yuyv_image, int width, int height, bool grayScale )
{
    // return image array
    unsigned char* rgb_image = new unsigned char[width * height * 3];

    int Y, U, V;
    unsigned char R, G, B;

    for( int i = 0, j = 0; i < width * height * 3; i+=6, j+=4 )
    {
        //first pixel
        Y = yuyv_image[j];
        V = yuyv_image[j+1];
        U = yuyv_image[j+3];

        R = R_fromYUV( Y, U, V );
        G = R;
        B = R;
        if( !grayScale )
        {
            G = G_fromYUV( Y, U, V );
            B = B_fromYUV( Y, U, V );
        }

        rgb_image[i] = R;
        rgb_image[i+1] = G;
        rgb_image[i+2] = B;

        //second pixel
        Y = yuyv_image[j+2];
        V = yuyv_image[j+1];
        U = yuyv_image[j+3];

        R = R_fromYUV( Y, U, V );
        G = R;
        B = R;
        if( !grayScale )
        {
            G = G_fromYUV( Y, U, V );
            B = B_fromYUV( Y, U, V );
        }

        rgb_image[i+3] = R;
        rgb_image[i+4] = G;
        rgb_image[i+5] = B;
    }

    return rgb_image;
}

unsigned char * yuv422ToRGB( unsigned char * yuyv_image, int width, int height, bool grayScale )
{
    // return image array
    unsigned char* rgb_image = new unsigned char[width * height * 3];

    int Y, U, V;
    unsigned char R, G, B;

    for( int i = 0, j = 0; i < width * height * 3; i+=6, j+=4 )
    {
        //first pixel
        Y = yuyv_image[j];
        U = yuyv_image[j+1];
        V = yuyv_image[j+3];

        R = R_fromYUV( Y, U, V );
        G = R;
        B = R;
        if( !grayScale )
        {
            G = G_fromYUV( Y, U, V );
            B = B_fromYUV( Y, U, V );
        }

        rgb_image[i] = R;
        rgb_image[i+1] = G;
        rgb_image[i+2] = B;

        //second pixel
        Y = yuyv_image[j+2];
        U = yuyv_image[j+1];
        V = yuyv_image[j+3];

        R = R_fromYUV( Y, U, V );
        G = R;
        B = R;
        if( !grayScale )
        {
            G = G_fromYUV( Y, U, V );
            B = B_fromYUV( Y, U, V );
        }

        rgb_image[i+3] = R;
        rgb_image[i+4] = G;
        rgb_image[i+5] = B;
    }

    return rgb_image;
}

unsigned char clamp(double value, double min, double max)
{
    if (value < min)
        return static_cast<unsigned char>(min);
    if (value > max)
        return static_cast<unsigned char>(max);
    return static_cast<unsigned char>(value);
}

unsigned char* yuy2422ToRGB(unsigned char* yuy2Data, int width, int height, bool grayScale)
{
    // return image array
    unsigned char* rgbData = new unsigned char[width * height * 3];

    int rgbIndex = 0;
    for (int i = 0; i < width * height * 2; i += 4)
    {
        unsigned char y1 = yuy2Data[i];
        unsigned char u = yuy2Data[i + 1];
        unsigned char y2 = yuy2Data[i + 2];
        unsigned char v = yuy2Data[i + 3];

        rgbData[rgbIndex++] = clamp(y1 + 1.772 * (u - 128), 0.0, 255.0);
        rgbData[rgbIndex++] = clamp(y1 - 0.344136 * (u - 128) - 0.714136 * (v - 128), 0.0, 255.0);
        rgbData[rgbIndex++] = clamp(y1 + 1.402 * (v - 128), 0.0, 255.0);

        rgbData[rgbIndex++] = clamp(y2 + 1.772 * (u - 128), 0.0, 255.0);
        rgbData[rgbIndex++] = clamp(y2 - 0.344136 * (u - 128) - 0.714136 * (v - 128), 0.0, 255.0);
        rgbData[rgbIndex++] = clamp(y2 + 1.402 * (v - 128), 0.0, 255.0);
    }

    return rgbData;
}