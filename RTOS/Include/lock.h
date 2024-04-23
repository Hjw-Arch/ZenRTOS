#ifndef LOCK_H
#define LOCK_H
#include <stdint.h>

extern uint8_t schedLockCount;

uint32_t enterCritical(void);
void leaveCritical(uint32_t status);



void schedLockInit(void);
void lockSched(void);
void unlockSched(void);

#endif
