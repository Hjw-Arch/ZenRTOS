#ifndef EXIO_H
#define EXIO_H

#include <stdint.h>
#include "bitband.h"

typedef enum {
	IO_LOW = 0,
	IO_HIGH,
}IOState;

typedef enum {
	IO_BEEP = 8,
	IO_LED1 = 9,
	IO_LED2 = 10,
}IOType;


#define		BEEP			PFout(8)
#define 	LED1			PFout(9)
#define 	LED2			PFout(10)

void ExIOInit(void);
void ExIOSetState(IOType type, IOState st);
IOState ExIOGetState(IOType type);




#endif
