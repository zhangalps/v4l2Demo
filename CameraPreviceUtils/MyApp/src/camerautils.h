#ifndef __CAM_UTILS__
#define __CAM_UTILS__

#include <dlfcn.h>
#include <stddef.h>

#include <stdlib.h>

typedef unsigned char BYTE;

void YUY2_RGB(BYTE *YUY2buff, BYTE *RGBbuff, size_t dwSize);

#endif //__CAM_UTILS__

