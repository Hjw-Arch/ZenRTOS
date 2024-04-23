#include "rtLib.h"
#include <assert.h>

void bitmapInit(Bitmap* bitmap) {
	bitmap->bitmap = 0;
}

void bitmapSet(Bitmap* bitmap, uint32_t pos) {
	assert(pos < 32);
	bitmap->bitmap |= 1 << pos;
}

void bitmapClear(Bitmap* bitmap, uint32_t pos) {
	assert(pos < 32);
	bitmap->bitmap &= ~(1 << pos);
}

uint32_t bitmapPosCount() {
	return 32;
}

uint8_t bitmapIsSet(Bitmap* bitmap, uint32_t pos) {
	return ((bitmap->bitmap & (1 << pos)) ? 1 : 0);
}

uint32_t bitmapGetFirstSet(Bitmap* bitmap) {
	static const uint8_t quickFindTable[] = {
		/* 00 */ 0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 10 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 20 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 30 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 40 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 50 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 60 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 70 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 80 */ 7,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 90 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* A0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* B0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* C0 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* D0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* E0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* F0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
	};
	
	if (bitmap->bitmap & 0xff) {
		return quickFindTable[bitmap->bitmap & 0xff];
	}
	else if (bitmap->bitmap & 0xff00) {
		return quickFindTable[(bitmap->bitmap >> 8) & 0xff] + 8;
	}
	else if (bitmap->bitmap & 0xff0000) {
		return quickFindTable[(bitmap->bitmap >> 16) & 0xff] + 16;
	}
	else if (bitmap->bitmap & 0xff000000) {
		return quickFindTable[(bitmap->bitmap >> 24) & 0xff] + 24;
	}
	else {
		return bitmapPosCount();
	}
}
















