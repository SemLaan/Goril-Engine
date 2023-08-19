#pragma once
#include <cstdint>

#ifdef GR_DLL
#define GRAPI __declspec(dllexport)
#else
#define GRAPI __declspec(dllimport)
#endif

#ifdef _MSC_VER // Stupid compiler doesn't think I know what i'm doing
#pragma warning(disable : 4996)
#pragma warning(disable : 6386)
#endif


// Unsigned int types.
typedef std::uint8_t u8;
typedef std::uint16_t u16;
typedef std::uint32_t u32;
typedef std::uint64_t u64;

// Signed int types.
typedef std::int8_t i8;
typedef std::int16_t i16;
typedef std::int32_t i32;
typedef std::int64_t i64;

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef std::int32_t b32;
typedef bool b8;

#define KiB ((size_t)1024u)
#define MiB ((size_t)1024u * (size_t)1024u)
#define GiB ((size_t)1024u * (size_t)1024u * (size_t)1024u)