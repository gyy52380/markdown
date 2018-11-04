#pragma once

#include <stdio.h>

#include "file_io.h"
#include "memory.h"
#include "string.h"

bool read_entire_file(String* data, String path)
{
    char* c_path = make_c_style_string(path);

    // Slightly inefficient što moraš allocateat C-style null-terminated string,
    // ali budući da fopen() u pozadini poziva Windowsov CreateFileW() koji zahtjeva
    // conversion u UTF-16 encoding, korištenje fopen() je inefficient anyway.

#if 0
    // Brži, Windows-specific alternative.
    String16 path16 = convert_utf8_to_utf16(path);
    HANDLE file = CreateFileW(path16.data, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
#endif

    FILE* f = fopen(c_path, "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    u64 length = ftell(f);
    fseek(f, 0, SEEK_SET);

    data->length = length;
    data->data = (u8*) lk_region_alloc(&temporary_memory, length,1);
    int count_read = fread(data->data, length, 1, f);
    fclose(f);

    if (count_read != 1)
        return false;
    return true;
}