#include "image_utils.h"

unsigned char R_fromYUV( int Y, int U, int V )
{
    double R = Y + (1.4065 * (U - 128));

    // This prevents colour distortions in your rgb image
    if (R < 0) R = 0;
    else if (R > 255) R = 255;

    return (unsigned char)R;
}

unsigned char G_fromYUV( int Y, int U, int V )
{
    double G = Y - (0.3455 * (V - 128)) - (0.7169 * (U - 128));

    // This prevents colour distortions in your rgb image
    if (G < 0) G = 0;
    else if (G > 255) G = 255;

    return (unsigned char)G;
}

unsigned char B_fromYUV( int Y, int U, int V )
{
    double B = Y + (1.7790 * (V - 128));

    // This prevents colour distortions in your rgb image
    if (B < 0) B = 0;
    else if (B > 255) B = 255;

    return (unsigned char)B;
}