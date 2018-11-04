#pragma once

// Defer
template <typename F>
struct Defer_RAII
{
    F f;
    Defer_RAII(F f): f(f) {}
    ~Defer_RAII() { f(); }
};

template <typename F>
Defer_RAII<F> defer_function(F f)
{
    return Defer_RAII<F>(f);
}


#define Concatenate__(x, y) x##y
#define Concatenate_(x, y)  Concatenate__(x, y)
#define Concatenate(x, y)   Concatenate_(x, y)


#define Defer(code)   auto Concatenate(_defer_, __COUNTER__) = defer_function([&] () { code; })

// Memory manipulation
#include <memory.h>

#define ZeroStruct(address)    memset((address), 0, sizeof(*(address)));
#define ZeroStaticArray(array) memset((array),   0, sizeof((array)));

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))
#define ElementSize(array) sizeof((array)[0])

#define MemberSize(Type, member) sizeof(((Type*) 0)->member)