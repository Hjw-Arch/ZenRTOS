#ifndef RT_TIME_H
#define RT_TIME_H

#include <stdint.h>
#include "rtLib.h"

extern listHead rtTaskDelayList;

void taskDelay (uint32_t ms);

void setSysTick(uint32_t ms);

#endif
