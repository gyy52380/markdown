#pragma once

#include <stdlib.h>

#include "typedef.h"
#include "macros.h"
#include "memory.h"
#include "string.h"


//
// Memory, character and C-style string utilities.
//


void copy(void* to, const void* from, umm length)
{
    u8* to_bytes   = (u8*) to;
    u8* from_bytes = (u8*) from;
    for (umm i = 0; i < length; i++)
        *(to_bytes++) = *(from_bytes++);
}

void move(void* to, void* from, umm length)
{
    if (to < from)
    {
        u8* to_bytes   = (u8*) to;
        u8* from_bytes = (u8*) from;
        for (umm i = 0; i < length; i++)
            *(to_bytes++) = *(from_bytes++);
    }
    else
    {
        u8* to_bytes   = (u8*) to   + length;
        u8* from_bytes = (u8*) from + length;
        for (umm i = 0; i < length; i++)
            *(--to_bytes) = *(--from_bytes);
    }
}

bool compare(const void* m1, const void* m2, umm length)
{
    const u8* bytes1 = (const u8*) m1;
    const u8* bytes2 = (const u8*) m2;

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

bool is_decimal_digit(u8 character)
{
    return character >= '0' && character <= '9';
}

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
// 8-bit strings.
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

String wrap_string(const char* c_string)
{
    String result;
    result.length = length_of_c_style_string(c_string);
    result.data = (u8*) c_string;
    return result;
}


String allocate_string(Region* memory, String string)
{
    String result;
    result.length = string.length;
    result.data = LK_RegionArray(memory, u8, string.length);

    copy(result.data, string.data, string.length);

    return result;
}


String clone_string(String string)
{
    return allocate_string(temp, string);
}


String concatenate(String first, String second, String third, String fourth, String fifth, String sixth)
{
    String result;
    result.length = first.length + second.length + third.length + fourth.length + fifth.length + sixth.length;
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

    copy(write, sixth.data, sixth.length);
    write += sixth.length;

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


bool prefix_equals(String string, String prefix)
{
    if (string.length < prefix.length)
        return false;

    return compare(string.data, prefix.data, prefix.length);
}

bool suffix_equals(String string, String suffix)
{
    if (string.length < suffix.length)
        return false;

    u8* substring = string.data + string.length - suffix.length;
    return compare(substring, suffix.data, suffix.length);
}


umm find_first_occurance(String string, u8 of)
{
    for (imm i = 0; i < string.length; i++)
        if (string[i] == of)
            return i;

    return NOT_FOUND;
}

umm find_first_occurance(String string, String of)
{
    if (string.length < of.length)
        return NOT_FOUND;

    for (imm i = 0; i <= string.length - of.length; i++)
        if (compare(string.data + i, of.data, of.length))
            return i;

    return NOT_FOUND;
}

umm find_first_occurance_of_any(String string, String any_of)
{
    for (imm i = 0; i < string.length; i++)
    {
        u8 c = string[i];

        for (umm j = 0; j < any_of.length; j++)
        {
            u8 c2 = any_of[j];
            if (c == c2)
                return i;
        }
    }

    return NOT_FOUND;
}


umm find_last_occurance(String string, u8 of)
{
    for (imm i = string.length - 1; i >= 0; i--)
        if (string[i] == of)
            return i;

    return NOT_FOUND;
}

umm find_last_occurance(String string, String of)
{
    if (string.length < of.length)
        return NOT_FOUND;

    for (imm i = string.length - of.length; i >= 0; i--)
        if (compare(string.data + i, of.data, of.length))
            return i;

    return NOT_FOUND;
}

umm find_last_occurance_of_any(String string, String any_of)
{
    for (imm i = string.length - 1; i >= 0; i--)
    {
        u8 c = string[i];

        for (umm j = 0; j < any_of.length; j++)
        {
            u8 c2 = any_of[j];
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
    u32 crc = U32_MAX;

    for (umm i = 0; i < data.length; i++)
    {
        crc = crc ^ data.data[i];
        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
    }

    return ~crc;
}


//
//
// Text reading utilities.
//
//


void consume(String* string, umm amount)
{
    DebugAssert(amount <= string->length);
    string->data += amount;
    string->length -= amount;
}

void consume_whitespace(String* string)
{
    while (*string && is_whitespace(string->data[0]))
        consume(string, 1);
}

String consume_line(String* string)
{
    consume_whitespace(string);

    umm line_length = find_first_occurance_of_any(*string, line_ending_chars);
    if (line_length == NOT_FOUND)
        line_length = string->length;

    String line = substring(*string, 0, line_length);
    consume(string, line_length);

    return line;
}

String consume_line_preserve_whitespace(String* string)
{
    umm line_length = find_first_occurance_of_any(*string, "\n\r"_s);
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
            // Handle two-u8 line endings.
            u8 c1 = string->data[0];
            u8 c2 = string->data[1];
            if ((c1 == '\n' && c2 == '\r') ||
                (c1 == '\r' && c2 == '\n'))
                ending_length++;
        }
        consume(string, ending_length);
    }

    return line;
}

String peek_line_preserve_whitespace(String string)
{
    umm line_length = find_first_occurance_of_any(string, "\n\r"_s);
    if (line_length == NOT_FOUND)
        line_length = string.length;

    String line = substring(string, 0, line_length);
    consume(&string, line_length);

    // If we've found the line ending, consume it.
    if (string)
    {
        umm ending_length = 1;
        if (string.length > 1)
        {
            // Handle two-u8 line endings.
            u8 c1 = string.data[0];
            u8 c2 = string.data[1];
            if ((c1 == '\n' && c2 == '\r') ||
                (c1 == '\r' && c2 == '\n'))
                ending_length++;
        }
        consume(&string, ending_length);
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
        consume(string, until_what.length);

    return left;
}

String consume_until_any(String* string, String until_what)
{
    consume_whitespace(string);

    umm left_length = find_first_occurance_of_any(*string, until_what);
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
    return consume_until_any(string, whitespace_chars);
}


String trim(String string)
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

bool read_u16be(String* string, u16* result) { Defer(*result = endian_swap16(*result)); return read_bytes(string, result, 2); }
bool read_u32be(String* string, u32* result) { Defer(*result = endian_swap32(*result)); return read_bytes(string, result, 4); }
bool read_u64be(String* string, u64* result) { Defer(*result = endian_swap64(*result)); return read_bytes(string, result, 8); }

bool read_i16le(String* string, i16* result) { return read_bytes(string, result, 2); }
bool read_i32le(String* string, i32* result) { return read_bytes(string, result, 4); }
bool read_i64le(String* string, i64* result) { return read_bytes(string, result, 8); }

bool read_i16be(String* string, i16* result) { Defer(*result = (i16) endian_swap16((u16) *result)); return read_bytes(string, result, 2); }
bool read_i32be(String* string, i32* result) { Defer(*result = (i32) endian_swap32((u32) *result)); return read_bytes(string, result, 4); }
bool read_i64be(String* string, i64* result) { Defer(*result = (i64) endian_swap64((u64) *result)); return read_bytes(string, result, 8); }


//
// File path utilities.
//


String get_file_name(String path)
{
    umm last_slash_index = find_last_occurance_of_any(path, slash_chars);
    if (last_slash_index == NOT_FOUND)
        last_slash_index = 0;

    umm start_index = last_slash_index + 1;
    umm length = path.length - start_index;

    String file_name = substring(path, start_index, length);
    return file_name;
}


String get_file_name_without_extension(String path)
{
    umm last_slash_index = find_last_occurance_of_any(path, slash_chars);
    if (last_slash_index != NOT_FOUND)
        consume(&path, last_slash_index + 1);

    umm last_dot_index = find_last_occurance(path, '.');
    if (last_dot_index != NOT_FOUND)
        path.length = last_dot_index;

    return path;
}


String get_parent_directory_path(String path)
{
    umm last_slash_index = find_last_occurance_of_any(path, slash_chars);
    // @Incomplete if (last_slash_index == NOT_FOUND)

    String parent = substring(path, 0, last_slash_index);
    return parent;
}



//
// String building utilities.
//



void free_string_builder(String_Builder* builder)
{
    free(builder->string.data);
    ZeroStruct(builder);
}


void clear(String_Builder* builder)
{
    builder->string.length = 0;
}


void maybe_resize(String_Builder* builder, umm insert_length)
{
    umm new_length = builder->string.length + insert_length;
    if (new_length >= builder->capacity)
    {
        umm new_capacity = builder->capacity;
        if (!new_capacity) new_capacity = 64;

        // growth factor of 1.5
        do new_capacity = new_capacity + (new_capacity >> 1);
        while (new_length >= new_capacity);

        u8* new_string = (u8*) malloc(new_capacity);
        if (builder->string.data)
        {
            copy(new_string, builder->string.data, builder->string.length);
            free(builder->string.data);
        }

        builder->string.data = new_string;
        builder->capacity = new_capacity;
    }
}


void append(String_Builder* builder, const void* data, umm length)
{
    maybe_resize(builder, length);
    copy(builder->string.data + builder->string.length, data, length);
    builder->string.length += length;
    builder->string.data[builder->string.length] = 0;
}


void insert(String_Builder* builder, umm at_offset, const void* data, umm length)
{
    maybe_resize(builder, length);

    umm string_length = builder->string.length;
    DebugAssert(at_offset <= string_length);
    builder->string.length += length;

    u8* from = builder->string.data + at_offset;
    u8* to = from + length;
    umm move_size = string_length - at_offset + 1;

    move(to, from, move_size);
    copy(from, data, length);
}


void remove(String_Builder* builder, umm at_offset, umm length)
{
    umm string_length = builder->string.length;
    DebugAssert(at_offset <= string_length);
    DebugAssert(at_offset + length <= string_length);
    builder->string.length -= length;

    u8* to = builder->string.data + at_offset;
    u8* from = to + length;
    umm move_size = string_length - at_offset - length + 1;
    move(to, from, move_size);
}



