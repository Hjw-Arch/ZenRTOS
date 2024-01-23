#ifndef LOCK_H
#define LOCK_H
#include <stdint.h>

uint32_t enterCritical(void);
void leaveCritical(uint32_t status);

#endif
