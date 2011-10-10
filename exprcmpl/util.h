#ifndef _UTIL_H
#define _UTIL_H

#define _USE_MATH_DEFINES
#include <cmath>

#include "exprcmpl.h"

#include "ByteBuffer.h"

#define _ENABLE_EXPR_TOSTRING
#define _ENABLE_EXPR_EMIT

#ifdef _ENABLE_EXPR_TOSTRING
# include <stdio.h>
# include <Windows.h>
#endif

#define NULL 0

inline bool is_char(int32 c) { return (uint32(c) & (uint32(-1) << 8)) == 0; }
inline bool is_whitespace_char(int32 c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
inline bool is_lcletter_char(int32 c) { return c >= 'a' && c <= 'z'; STATIC_ASSERT('a' < 'z', "Char failure"); }
inline bool is_ucletter_char(int32 c) { return c >= 'A' && c <= 'Z'; STATIC_ASSERT('A' < 'Z', "Char failure"); }
inline bool is_digit_char(int32 c) { return c >= '0' && c <= '9'; STATIC_ASSERT('0' < '9', "Char failure"); }
inline bool is_letter_char(int32 c) { return is_lcletter_char(c) || is_ucletter_char(c); }
inline bool is_identifier_char(int32 c) { return is_letter_char(c) || is_digit_char(c) || c == '_'; }

inline bool eqdbl(double one, double two)
{
    return fabs(one - two) < 0.000000001;
}

inline void memcopy(uint8* dst, const uint8* src, int len)
{
    memcpy(dst, src, len);
}

#endif
