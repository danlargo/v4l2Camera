#ifndef V4L2_DEFS_H
#define V4L2_DEFS_H

#include <string>

void fourcc_int_to_charArray( unsigned int fourcc, char * ret );
unsigned int fourcc_charArray_to_int( unsigned char * fourcc );
unsigned int fourcc_intArray_to_int( int * fourcc );
std::string fourcc_int_to_descriptor( unsigned int fourcc );

#endif // V4L2_DEFS_H