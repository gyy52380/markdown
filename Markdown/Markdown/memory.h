#ifndef TEMP_MEMORY_HEADER
#define TEMP_MEMORY_HEADER

// header
#include "lk_region.h"

typedef LK_Region Region;

extern LK_Region temporary_memory;
constexpr LK_Region* temp = &temporary_memory;

#endif

#ifdef TEMP_MEMORY_IMPLEMENTATION
#ifndef TEMP_MEMORY_IMPLEMENTED
#define TEMP_MEMORY_IMPLEMENTED

// implementation
#define LK_REGION_IMPLEMENTATION
#include "lk_region.h"
LK_Region temporary_memory = {};

#endif
#endif