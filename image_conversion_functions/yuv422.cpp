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