#pragma strict_gs_check(on)

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>

#undef near
#undef far

#pragma comment(lib, "Shlwapi.lib")


#include "string.h"


//
// Memory, character and C-style string utilities.
//


void copy(void* to, const void* from, umm length)
{
    byte* to_bytes   = (byte*) to;
    byte* from_bytes = (byte*) from;

    for (umm i = 0; i < length; i++)
    {
        *to_bytes = *from_bytes;

        to_bytes++;
        from_bytes++;
    }
}

bool compare(const void* m1, const void* m2, umm length)
{
    const byte* bytes1 = (const byte*) m1;
    const byte* bytes2 = (const byte*) m2;

    for (umm i = 0; i < length; i++)
    {
        if (*bytes1 != *bytes2)
            return false;

        bytes1++;
        bytes2++;
    }

    return true;
}


u16 endian_swap16(u16 value)
{
    return (value << 8) | (value >> 8);
}

u32 endian_swap32(u32 value)
{
    return (value << 24) | (value >> 24) |
           ((value << 8) & 0xff0000ul) |
           ((value >> 8) & 0x00ff00ul);
}

u64 endian_swap64(u64 value)
{
    u32 low32  = (u32) value;
    u32 high32 = (u32)(value >> 32);

    u32 new_low32  = endian_swap32(high32);
    u32 new_high32 = endian_swap32(low32);

    return (u64) new_low32 | ((u64) new_high32 << 32);
}


static String line_ending_chars = "\n\r"_s;
static String whitespace_chars = " \t\n\r"_s;
static String slash_chars = "/\\"_s;


bool is_whitespace(u8 character)
{
    if (character == ' ')  return true;
    if (character == '\t') return true;
    if (character == '\n') return true;
    if (character == '\r') return true;

    return false;
}

umm length_of_c_style_string(const char* c_string)
{
    umm length = 0;
    while (c_string[length])
        length++;
    return length;
}

umm length_of_c_style_string(const u16* c_string)
{
    umm length = 0;
    while (c_string[length])
        length++;
    return length;
}


//
// 8-bit character strings.
// When treated as text, UTF-8 encoding is assumed.
//


String make_string(const char* c_string)
{
    umm length = length_of_c_style_string(c_string);

    String result;
    result.length = length;
    result.data = LK_RegionArray(temp, u8, length);

    copy(result.data, c_string, length);

    return result;
}

char* make_c_style_string(String string)
{
    char* result = LK_RegionArray(temp, char, string.length + 1);

    copy(result, string.data, string.length);
    result[string.length] = 0;

    return result;
}


String clone_string(String string)
{
    String result;
    result.length = string.length;
    result.data = LK_RegionArray(temp, u8, string.length);

    copy(result.data, string.data, string.length);

    return result;
}


String concatenate(String first, String second, String third, String fourth, String fifth)
{
    String result;
    result.length = first.length + second.length + third.length + fourth.length + fifth.length;
    result.data = LK_RegionArray(temp, u8, result.length);

    u8* write = result.data;

    copy(write, first.data, first.length);
    write += first.length;

    copy(write, second.data, second.length);
    write += second.length;

    copy(write, third.data, third.length);
    write += third.length;

    copy(write, fourth.data, fourth.length);
    write += fourth.length;

    copy(write, fifth.data, fifth.length);
    write += fifth.length;

    return result;
}


String substring(String string, umm start_index, umm length)
{
    DebugAssert(start_index <= string.length);
    DebugAssert((start_index + length) <= string.length);

    String result;
    result.length = length;
    result.data = string.data + start_index;

    return result;
}


bool operator==(String lhs, String rhs)
{
    if (lhs.length != rhs.length)
        return false;
    return compare(lhs.data, rhs.data, lhs.length);
}

bool operator==(String lhs, const char* rhs)
{
    umm rhs_length = length_of_c_style_string(rhs);
    if (lhs.length != rhs_length)
        return false;
    return compare(lhs.data, rhs, lhs.length);
}

bool operator==(const char* lhs, String rhs)
{
    umm lhs_length = length_of_c_style_string(lhs);
    if (lhs_length != rhs.length)
        return false;
    return compare(lhs, rhs.data, lhs_length);
}


bool prefix_equals(String string, String suffix)
{
    if (string.length < suffix.length)
        return false;

    return compare(string.data, suffix.data, suffix.length);
}

bool suffix_equals(String string, String suffix)
{
    if (string.length < suffix.length)
        return false;

    u8* substring = string.data + string.length - suffix.length;
    return compare(substring, suffix.data, suffix.length);
}


umm find_first_occurance(String string, u8 character)
{
    for (imm i = 0; i < string.length; i++)
        if (string[i] == character)
            return i;

    return NOT_FOUND;
}

umm find_first_occurance(String string, String characters)
{
    for (imm i = 0; i < string.length; i++)
    {
        u8 c = string[i];

        for (umm j = 0; j < characters.length; j++)
        {
            u8 c2 = characters[j];
            if (c == c2)
                return i;
        }
    }

    return NOT_FOUND;
}


umm find_last_occurance(String string, u8 character)
{
    for (imm i = string.length - 1; i >= 0; i--)
        if (string[i] == character)
            return i;

    return NOT_FOUND;
}

umm find_last_occurance(String string, String characters)
{
    for (imm i = string.length - 1; i >= 0; i--)
    {
        u8 c = string[i];

        for (umm j = 0; j < characters.length; j++)
        {
            u8 c2 = characters[j];
            if (c == c2)
                return i;
        }
    }

    return NOT_FOUND;
}


void replace_all_occurances(String string, u8 what, u8 with_what)
{
    for (imm i = string.length - 1; i >= 0; i--)
        if (string[i] == what)
            string[i] = with_what;
}


u32 compute_crc32(String data)
{
    u32 crc = ~0ul;

    for (umm i = 0; i < data.length; i++)
    {
        crc = crc ^ data.data[i];
        crc = (crc >> 1) ^ (0xedb88320 & -(i32)(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(i32)(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(i32)(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(i32)(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(i32)(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(i32)(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(i32)(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(i32)(crc & 1));
    }

    return ~crc;
}


//
//
// Text reading utilities.
//
//


u8 consume(String* string, umm amount)
{
    DebugAssert(amount <= string->length);

    u8 return_first_byte = *(string->data);

    string->data += amount;
    string->length -= amount;

    return return_first_byte;
}

String trim(String string)
{
    while (string && is_whitespace(string[0]))
        consume(&string, 1);
    while (string && is_whitespace(string[string.length - 1]))
        string.length--;
    return string;
}

void consume_whitespace(String* string)
{
    while (*string && is_whitespace(string->data[0]))
        consume(string, 1);
}

String consume_line(String* string)
{
    consume_whitespace(string);

    umm line_length = find_first_occurance(*string, line_ending_chars);
    if (line_length == NOT_FOUND)
        line_length = string->length;

    String line = substring(*string, 0, line_length);
    consume(string, line_length);

    return line;
}

String consume_line_preserve_whitespace(String* string)
{
    umm line_length = find_first_occurance(*string, "\n\r"_s);
    if (line_length == NOT_FOUND)
        line_length = string->length;

    String line = substring(*string, 0, line_length);
    consume(string, line_length);

    // If we've found the line ending, consume it.
    if (*string)
    {
        umm ending_length = 1;
        if (string->length > 1)
        {
            // Handle two-byte line endings.
            char c1 = string->data[0];
            char c2 = string->data[1];
            if ((c1 == '\n' && c2 == '\r') || (c1 == '\r' && c2 == '\n'))
                ending_length++;
        }
        consume(string, ending_length);
    }

    return line;
}

String consume_until(String* string, u8 until_what)
{
    consume_whitespace(string);

    umm left_length = find_first_occurance(*string, until_what);
    if (left_length == NOT_FOUND)
        left_length = string->length;

    String left = substring(*string, 0, left_length);
    consume(string, left_length);

    // If we've found the delimiter, consume it.
    if (*string)
        consume(string, 1);

    return left;
}

String consume_until(String* string, String until_what)
{
    consume_whitespace(string);

    umm left_length = find_first_occurance(*string, until_what);
    if (left_length == NOT_FOUND)
        left_length = string->length;

    String left = substring(*string, 0, left_length);
    consume(string, left_length);

    // If we've found the delimiter, consume it.
    if (*string)
        consume(string, 1);

    return left;
}

String consume_until_whitespace(String* string)
{
    return consume_until(string, whitespace_chars);
}


String ewltrim(String string)
{
    while (string && is_whitespace(string[0]))
        consume(&string, 1);
    while (string && is_whitespace(string[string.length - 1]))
        string.length--;
    return string;
}


//
//
// Binary reading utilities.
//
//


bool read_bytes(String* string, void* result, umm count)
{
    if (string->length < count)
        return false;

    copy(result, string->data, count);
    consume(string, count);
    return true;
}


bool read_u8 (String* string, u8*  result) { return read_bytes(string, result, 1); }
bool read_u16(String* string, u16* result) { return read_bytes(string, result, 2); }
bool read_u32(String* string, u32* result) { return read_bytes(string, result, 4); }
bool read_u64(String* string, u64* result) { return read_bytes(string, result, 8); }

bool read_i8 (String* string, i8*  result) { return read_bytes(string, result, 1); }
bool read_i16(String* string, i16* result) { return read_bytes(string, result, 2); }
bool read_i32(String* string, i32* result) { return read_bytes(string, result, 4); }
bool read_i64(String* string, i64* result) { return read_bytes(string, result, 8); }


#ifdef BIG_ENDIAN
#error "Go away, we don't like you!"
#endif

bool read_u16le(String* string, u16* result) { return read_bytes(string, result, 2); }
bool read_u32le(String* string, u32* result) { return read_bytes(string, result, 4); }
bool read_u64le(String* string, u64* result) { return read_bytes(string, result, 8); }

bool read_u16be(String* string, u16* result) { bool success = read_bytes(string, result, 2); *result = endian_swap16(*result); return success; }
bool read_u32be(String* string, u32* result) { bool success = read_bytes(string, result, 4); *result = endian_swap32(*result); return success; }
bool read_u64be(String* string, u64* result) { bool success = read_bytes(string, result, 8); *result = endian_swap64(*result); return success; }

bool read_i16le(String* string, i16* result) { return read_bytes(string, result, 2); }
bool read_i32le(String* string, i32* result) { return read_bytes(string, result, 4); }
bool read_i64le(String* string, i64* result) { return read_bytes(string, result, 8); }

bool read_i16be(String* string, i16* result) { bool success = read_bytes(string, result, 2); *result = (i16) endian_swap16((u16) *result); return success; }
bool read_i32be(String* string, i32* result) { bool success = read_bytes(string, result, 4); *result = (i32) endian_swap32((u32) *result); return success; }
bool read_i64be(String* string, i64* result) { bool success = read_bytes(string, result, 8); *result = (i64) endian_swap64((u64) *result); return success; }


//
// File path utilities.
//


String get_file_name(String path)
{
    umm last_slash_index = find_last_occurance(path, slash_chars);
    if (last_slash_index == NOT_FOUND)
        last_slash_index = 0;

    umm start_index = last_slash_index + 1;
    umm length = path.length - start_index;

    String file_name = substring(path, start_index, length);
    return file_name;
}


String get_file_name_without_extension(String path)
{
    umm last_slash_index = find_last_occurance(path, slash_chars);
    if (last_slash_index == NOT_FOUND)
        last_slash_index = 0;

    umm last_dot_index = find_last_occurance(path, '.');
    if (last_dot_index == NOT_FOUND || last_dot_index < last_slash_index)
        last_dot_index = path.length;

    umm start_index = last_slash_index + 1;
    umm length = last_dot_index - start_index;

    String file_name = substring(path, start_index, length);
    return file_name;
}


String get_parent_directory_path(String path)
{
    umm last_slash_index = find_last_occurance(path, slash_chars);
    // @Incomplete if (last_slash_index == NOT_FOUND)

    String parent = substring(path, 0, last_slash_index);
    return parent;
}

bool is_path_relative(String path)
{
    // @Incomplete - Don't use Windows crap for this....
    String16 path16 = convert_utf8_to_utf16(path);
    return PathIsRelativeW((LPCWSTR) path16.data);
}


String maybe_make_path_absolute(String relative_path, String absolute_parent_directory)
{
    String result;

    if (!is_path_relative(relative_path))
    {
        result = clone_string(relative_path);
    }
    else
    {
        // @Incomplete - Different behavior on other operating systems.
        result = concatenate(absolute_parent_directory, "\\"_s, relative_path);
    }

    // @Incomplete - Different behavior on other operating systems.
    // Replace slashes with backslashes.

    replace_all_occurances(result, '/', '\\');

    // @Incomplete - Make sure that the path is valid and doesn't end with a slash.

    return result;
}


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



//
// 16-bit character strings. These mostly exist for interfacing with Windows.
// That's why conversion routines return null terminated strings, and why
// we don't really support any operations on them.
//


// The returned string is null terminated.
String16 make_string16(const u16* c_string)
{
    umm length = length_of_c_style_string(c_string);

    String16 result;
    result.length = length;
    result.data = LK_RegionArray(temp, u16, length + 1);

    copy(result.data, c_string, 2 * (length + 1));

    return result;
}

// The returned string is null terminated.
String16 convert_utf8_to_utf16(String string)
{
    String16 result;

    // @Incomplete - Don't use Windows crap for this....
    result.length = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR) string.data, string.length, NULL, 0);
    result.data = LK_RegionArray(temp, u16, result.length + 1);

    MultiByteToWideChar(CP_UTF8, 0, (LPCSTR) string.data, string.length, (LPWSTR) result.data, result.length);
    result.data[result.length] = 0;

    return result;
}

// The returned string is null terminated.
String convert_utf16_to_utf8(String16 string)
{
    String result;

    // @Incomplete - Don't use Windows crap for this....
    result.length = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH) string.data, string.length, NULL, 0, 0, 0);
    result.data = LK_RegionArray(temp, u8, result.length + 1);

    WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR) string.data, string.length, (LPSTR) result.data, result.length, 0, 0);
    result.data[result.length] = 0;

    return result;
}



