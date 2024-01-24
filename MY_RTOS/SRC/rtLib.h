#ifndef RTLIB_H
#define RTLIB_H

#include <stdint.h>

typedef struct {
	uint32_t bitmap;
}Bitmap;

void bitmapInit(Bitmap* bitmap);
void bitmapSet(Bitmap* bitmap, uint32_t pos);
void bitmapClear(Bitmap* bitmap, uint32_t pos);
uint32_t bitmapPosCount(void);
uint32_t bitmapGetFirstSet(Bitmap* bitmap);

#endif
