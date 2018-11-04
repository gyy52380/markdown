#pragma once

#ifdef TEMP_MEMORY_IMPLEMENTATION

#define LK_REGION_IMPLEMENTATION
#include "lk_region.h"
LK_Region temporary_memory = {};

#else

#include "lk_region.h"
extern LK_Region temporary_memory;

#endif

constexpr LK_Region* temp = &temporary_memory;

typedef LK_Region Region;