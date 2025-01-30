#include <string>

unsigned char R_fromYUV( int Y, int U, int V );
unsigned char G_fromYUV( int Y, int U, int V );
unsigned char B_fromYUV( int Y, int U, int V );

unsigned char * gs16ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale );
unsigned char * gs8ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale );

unsigned char * interleavedYUV420ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale );
unsigned char * interleavedYVU420ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale );

unsigned char * planarYUV420ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale );
unsigned char * planarYVU420ToRGB( unsigned char * yuv_image, int width, int height, bool grayScale );

bool saveRGB24AsBMP( unsigned char * rgbData, int width, int height, std::string fid );

unsigned char * yvu422ToRGB( unsigned char * yuyv_image, int width, int height, bool grayScale );
unsigned char * yuv422ToRGB( unsigned char * yuyv_image, int width, int height, bool grayScale );
unsigned char * yuy2422ToRGB( unsigned char * yuyv_image, int width, int height, bool grayScale );

