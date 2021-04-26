#pragma once
#include <cstdint>
#include <stdint.h>
#include <cinttypes>
#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdarg>

#define NOT_VALID  -1

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86))
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

//=============================================================================================================================================================================
// Byte swap
//=============================================================================================================================================================================
#if defined(_MSC_VER)
static inline uint16_t xSwapBytes16(uint16_t Value) { return _byteswap_ushort(Value); }
static inline  int16_t xSwapBytes16( int16_t Value) { return _byteswap_ushort(Value); }
static inline uint32_t xSwapBytes32(uint32_t Value) { return _byteswap_ulong (Value); }
static inline  int32_t xSwapBytes32( int32_t Value) { return _byteswap_ulong (Value); }
static inline uint64_t xSwapBytes64(uint64_t Value) { return _byteswap_uint64(Value); }
static inline  int64_t xSwapBytes64( int64_t Value) { return _byteswap_uint64(Value); }
#elif defined (__GNUC__)
static inline uint16_t xSwapBytes16(uint16_t Value) { return __builtin_bswap16(Value); }
static inline  int16_t xSwapBytes16( int16_t Value) { return __builtin_bswap16(Value); }
static inline uint32_t xSwapBytes32(uint32_t Value) { return __builtin_bswap32(Value); }
static inline  int32_t xSwapBytes32( int32_t Value) { return __builtin_bswap32(Value); }
static inline uint64_t xSwapBytes64(uint64_t Value) { return __builtin_bswap64(Value); }
static inline  int64_t xSwapBytes64( int64_t Value) { return __builtin_bswap64(Value); }
#else
#error Unrecognized compiler
#endif




uint64_t convertFrom8To64(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,
    uint8_t b5, uint8_t b6, uint8_t b7, uint8_t b8) 

{
    uint64_t result = 0x0000;

    result = b1;
    result = result << 8;
    result |= b2;
    result = result << 8;
    result |= b3;
    result = result << 8;
    result |= b4;
    result = result << 8;
    result |= b5;
    result = result << 8;
    result |= b6;
    result = result << 8;
    result |= b7;
    result = result << 8;
    result |= b8;

    return result;
}

uint32_t convertFrom8To32(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth) {
    uint32_t result = 0x0000;

    result = first;
    result = result << 8;
    result |= second;
    result = result << 8;
    result |= third;
    result = result << 8;
    result |= fourth;

    return result;
}
uint32_t convertFrom8To24(uint8_t first, uint8_t second, uint8_t third) {
    uint32_t result = 0x0000;

    result = first;
    result = result << 8;
    result |= second;
    result = result << 8;
    result |= third;

    return result;
}


uint16_t convertFrom8To16(uint8_t hi, uint8_t lo) {
    uint16_t result = 0x0000;

    result = hi;
    result = result << 8;
    result |= lo;
    return result;
}



template<class T>
T connectBinaryWords(uint8_t num ...) {
    T result = 0x0000;

    va_list ap;
    va_start(ap, num);
    result = va_arg(ap, uint8_t);

    for (int i = 1; i < num; i++) {
        result = result << 8;
        result |= va_arg(ap, uint8_t);

    }

    return result;
}