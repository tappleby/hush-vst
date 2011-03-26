/*
  WDL - wdlendian.h
  (c) Theo Niessink 2011
  <http://www.taletn.com/>

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.


  This file provides macros and functions for converting integer and
  floating point data types from native (host) endian to little or big
  endian format, and vice versa.

*/


#ifndef _WDL_ENDIAN_H_
#define _WDL_ENDIAN_H_


#include "wdltypes.h"

#ifdef __cplusplus
	#define WDL_ENDIAN_INLINE inline
#elif defined(_MSC_VER)
	#define WDL_ENDIAN_INLINE __inline
#else
	#define WDL_ENDIAN_INLINE
#endif


// Windows
#ifdef _WIN32
#define WDL_LITTLE_ENDIAN

// Mac OS X
#elif defined(__APPLE__)
#if __LITTLE_ENDIAN__ // Intel
	#define WDL_LITTLE_ENDIAN
#elif __BIG_ENDIAN__ // PowerPC
	#define WDL_BIG_ENDIAN
#else
	#error Unknown endian
#endif

// GNU C
#elif defined(__BYTE_ORDER__)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	#define WDL_LITTLE_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	#define WDL_BIG_ENDIAN
#else
	#error Unsupported endian
#endif
#if __FLOAT_WORD_ORDER__ != __BYTE_ORDER__
	#error Unsupported float endian
#endif

#else
#error Unknown endian
#endif


// Microsoft C
#ifdef _MSC_VER
#include <intrin.h>
#define WDL_bswap16(x) _byteswap_ushort(x)
#define WDL_bswap32(x) _byteswap_ulong(x)
#define WDL_bswap64(x) _byteswap_uint64(x)

// Mac OS X (v10.0 and later)
#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#define WDL_bswap16(x) Endian16_Swap(x)
#define WDL_bswap32(x) Endian32_Swap(x)
#define WDL_bswap64(x) Endian64_Swap(x) // (Thread-safe on) v10.3 and later (?)

// GNU C
#elif __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
#define WDL_bswap32(x) __builtin_bswap32(x)
#define WDL_bswap64(x) __builtin_bswap64(x)

// Linux
//#elif defined(__linux)
//#include <sys/endian.h>
//#define WDL_bswap16(x) bswap16(x)
//#define WDL_bswap32(x) bswap32(x)
//#define WDL_bswap64(x) bswap64(x)

#endif // WDL_bswapXX

// If none of the supported intrinsics were found, then revert to generic C
// byte swap functions.
#ifndef WDL_bswap16
static WDL_ENDIAN_INLINE unsigned short WDL_bswap16(const unsigned short int16)
{
	return int16 >> 8 |
	       int16 << 8;
}
#endif

#ifndef WDL_bswap32
static WDL_ENDIAN_INLINE unsigned int WDL_bswap32(const unsigned int int32)
{
	return int32 >> 24 |
	       int32 >> 8  & 0x0000FF00 |
	       int32 << 8  & 0x00FF0000 |
	       int32 << 24;
}
#endif

#ifndef WDL_bswap64
static WDL_ENDIAN_INLINE WDL_UINT64 WDL_bswap64(const WDL_UINT64 int64)
{
	return int64 >> 56 |
	       int64 >> 40 & 0x000000000000FF00ULL |
	       int64 >> 24 & 0x0000000000FF0000ULL |
	       int64 >> 8  & 0x00000000FF000000ULL |
	       int64 << 8  & 0x000000FF00000000ULL |
	       int64 << 24 & 0x0000FF0000000000ULL |
	       int64 << 40 & 0x00FF000000000000ULL |
	       int64 << 56;
}
#endif


// Mac OS X
#if defined(__APPLE__)
#define WDL_htole16(x) EndianU16_NtoL(x)
#define WDL_htole32(x) EndianU32_NtoL(x)
#define WDL_htole64(x) EndianU64_NtoL(x)
#define WDL_le16toh(x) EndianU16_LtoN(x)
#define WDL_le32toh(x) EndianU32_LtoN(x)
#define WDL_le64toh(x) EndianU64_LtoN(x)
#define WDL_htobe16(x) EndianU16_NtoB(x)
#define WDL_htobe32(x) EndianU32_NtoB(x)
#define WDL_htobe64(x) EndianU64_NtoB(x)
#define WDL_be16toh(x) EndianU16_BtoN(x)
#define WDL_be32toh(x) EndianU32_BtoN(x)
#define WDL_be64toh(x) EndianU64_BtoN(x)

// Linux
//#elif defined(__linux)
//#define WDL_htole16(x) htole16(x)
//#define WDL_htole32(x) htole32(x)
//#define WDL_htole64(x) htole64(x)
//#define WDL_le16toh(x) le16toh(x)
//#define WDL_le32toh(x) le32toh(x)
//#define WDL_le64toh(x) le64toh(x)
//#define WDL_htobe16(x) htobe16(x)
//#define WDL_htobe32(x) htobe32(x)
//#define WDL_htobe64(x) htobe64(x)
//#define WDL_be16toh(x) be16toh(x)
//#define WDL_be32toh(x) be32toh(x)
//#define WDL_be64toh(x) be64toh(x)

// Generic little-endian
#elif defined(WDL_LITTLE_ENDIAN)
#define WDL_htole16(x) ((unsigned short)(x))
#define WDL_htole32(x) ((unsigned int)(x))
#define WDL_htole64(x) ((WDL_UINT64)(x))
#define WDL_le16toh(x) ((unsigned short)(x))
#define WDL_le32toh(x) ((unsigned int)(x))
#define WDL_le64toh(x) ((WDL_UINT64)(x))
#define WDL_htobe16(x) WDL_bswap16(x)
#define WDL_htobe32(x) WDL_bswap32(x)
#define WDL_htobe64(x) WDL_bswap64(x)
#define WDL_be16toh(x) WDL_bswap16(x)
#define WDL_be32toh(x) WDL_bswap32(x)
#define WDL_be64toh(x) WDL_bswap64(x)

// Generic big-endian
#elif defined(WDL_BIG_ENDIAN)
#define WDL_htobe16(x) ((unsigned short)(x))
#define WDL_htobe32(x) ((unsigned int)(x))
#define WDL_htobe64(x) ((WDL_UINT64)(x))
#define WDL_be16toh(x) ((unsigned short)(x))
#define WDL_be32toh(x) ((unsigned int)(x))
#define WDL_be64toh(x) ((WDL_UINT64)(x))
#define WDL_htole16(x) WDL_bswap16(x)
#define WDL_htole32(x) WDL_bswap32(x)
#define WDL_htole64(x) WDL_bswap64(x)
#define WDL_le16toh(x) WDL_bswap16(x)
#define WDL_le32toh(x) WDL_bswap32(x)
#define WDL_le64toh(x) WDL_bswap64(x)

#endif // WDL_htoleXX et al


// Map floating point types to int types.

#ifdef WDL_LITTLE_ENDIAN

	static WDL_ENDIAN_INLINE unsigned int WDL_ftole32(const float        host32)   { return *(const unsigned int*)&host32; }
	static WDL_ENDIAN_INLINE WDL_UINT64   WDL_ftole64(const double       host64)   { return *(const WDL_UINT64  *)&host64; }
	static WDL_ENDIAN_INLINE float        WDL_le32tof(const unsigned int little32) { return *(const float       *)&little32; }
	static WDL_ENDIAN_INLINE double       WDL_le64tof(const WDL_UINT64   little64) { return *(const double      *)&little64; }
	static WDL_ENDIAN_INLINE unsigned int WDL_ftobe32(const float        host32)   { return WDL_htobe32(*(const unsigned int*)&host32); }
	static WDL_ENDIAN_INLINE WDL_UINT64   WDL_ftobe64(const double       host64)   { return WDL_htobe64(*(const WDL_UINT64  *)&host64); }

	static WDL_ENDIAN_INLINE float WDL_be32tof(const unsigned int big32)
	{
		const unsigned int host32 = WDL_be32toh(big32);
		return *(const float*)&host32;
	}

	static WDL_ENDIAN_INLINE double WDL_be64tof(const WDL_UINT64 big64)
	{
		const WDL_UINT64 host64 = WDL_be64toh(big64);
		return *(const double*)&host64;
	}

#elif defined(WDL_BIG_ENDIAN)

	static WDL_ENDIAN_INLINE unsigned int WDL_ftobe32(const float        host32) { return *(const unsigned int*)&host32; }
	static WDL_ENDIAN_INLINE WDL_UINT64   WDL_ftobe64(const double       host64) { return *(const WDL_UINT64  *)&host64; }
	static WDL_ENDIAN_INLINE float        WDL_be32tof(const unsigned int big32)  { return *(const float       *)&big32; }
	static WDL_ENDIAN_INLINE double       WDL_be64tof(const WDL_UINT64   big64)  { return *(const double      *)&big64; }
	static WDL_ENDIAN_INLINE unsigned int WDL_ftole32(const float        host32) { return WDL_htole32(*(const unsigned int*)&host32); }
	static WDL_ENDIAN_INLINE WDL_UINT64   WDL_ftole64(const double       host64) { return WDL_htole64(*(const WDL_UINT64  *)&host64); }

	static WDL_ENDIAN_INLINE float WDL_le32tof(const unsigned int little32)
	{
		const unsigned int host32 = WDL_le32toh(little32);
		return *(const float*)&host32;
	}

	static WDL_ENDIAN_INLINE double WDL_le64tof(const WDL_UINT64 little64)
	{
		const WDL_UINT64 host64 = WDL_le64toh(little64);
		return *(const double*)&host64;
	}

#endif // WDL_ftoleXX et al


// C++ auto-typed wrappers
#ifdef __cplusplus

	static WDL_ENDIAN_INLINE unsigned short WDL_htole(unsigned short host16)   { return WDL_htole16(host16); }
	static WDL_ENDIAN_INLINE signed   short WDL_htole(signed   short host16)   { return WDL_htole16(host16); }
	static WDL_ENDIAN_INLINE unsigned int   WDL_htole(unsigned int   host32)   { return WDL_htole32(host32); }
	static WDL_ENDIAN_INLINE signed   int   WDL_htole(signed   int   host32)   { return WDL_htole32(host32); }
	static WDL_ENDIAN_INLINE WDL_UINT64     WDL_htole(WDL_UINT64     host64)   { return WDL_htole64(host64); }
	static WDL_ENDIAN_INLINE WDL_INT64      WDL_htole(WDL_INT64      host64)   { return WDL_htole64(host64); }

	static WDL_ENDIAN_INLINE unsigned int   WDL_ftole(float          host32)   { return WDL_ftole32(host32); }
	static WDL_ENDIAN_INLINE WDL_UINT64     WDL_ftole(double         host64)   { return WDL_ftole64(host64); }

	static WDL_ENDIAN_INLINE unsigned short WDL_letoh(unsigned short little16) { return WDL_le16toh(little16); }
	static WDL_ENDIAN_INLINE signed   short WDL_letoh(signed   short little16) { return WDL_le16toh(little16); }
	static WDL_ENDIAN_INLINE unsigned int   WDL_letoh(unsigned int   little32) { return WDL_le32toh(little32); }
	static WDL_ENDIAN_INLINE signed   int   WDL_letoh(signed   int   little32) { return WDL_le32toh(little32); }
	static WDL_ENDIAN_INLINE WDL_UINT64     WDL_letoh(WDL_UINT64     little64) { return WDL_le64toh(little64); }
	static WDL_ENDIAN_INLINE WDL_INT64      WDL_letoh(WDL_INT64      little64) { return WDL_le64toh(little64); }

	static WDL_ENDIAN_INLINE float          WDL_letof(unsigned int   little32) { return WDL_le32tof(little32); }
	static WDL_ENDIAN_INLINE double         WDL_letof(WDL_UINT64     little64) { return WDL_le64tof(little64); }

	static WDL_ENDIAN_INLINE unsigned short WDL_htobe(unsigned short host16)   { return WDL_htobe16(host16); }
	static WDL_ENDIAN_INLINE signed   short WDL_htobe(signed   short host16)   { return WDL_htobe16(host16); }
	static WDL_ENDIAN_INLINE unsigned int   WDL_htobe(unsigned int   host32)   { return WDL_htobe32(host32); }
	static WDL_ENDIAN_INLINE signed   int   WDL_htobe(signed   int   host32)   { return WDL_htobe32(host32); }
	static WDL_ENDIAN_INLINE WDL_UINT64     WDL_htobe(WDL_UINT64     host64)   { return WDL_htobe64(host64); }
	static WDL_ENDIAN_INLINE WDL_INT64      WDL_htobe(WDL_INT64      host64)   { return WDL_htobe64(host64); }

	static WDL_ENDIAN_INLINE unsigned int   WDL_ftobe(float          host32)   { return WDL_ftobe32(host32); }
	static WDL_ENDIAN_INLINE WDL_UINT64     WDL_ftobe(double         host64)   { return WDL_ftobe64(host64); }

	static WDL_ENDIAN_INLINE unsigned short WDL_betoh(unsigned short big16)    { return WDL_be16toh(big16); }
	static WDL_ENDIAN_INLINE signed   short WDL_betoh(signed   short big16)    { return WDL_be16toh(big16); }
	static WDL_ENDIAN_INLINE unsigned int   WDL_betoh(unsigned int   big32)    { return WDL_be32toh(big32); }
	static WDL_ENDIAN_INLINE signed   int   WDL_betoh(signed   int   big32)    { return WDL_be32toh(big32); }
	static WDL_ENDIAN_INLINE WDL_UINT64     WDL_betoh(WDL_UINT64     big64)    { return WDL_be64toh(big64); }
	static WDL_ENDIAN_INLINE WDL_INT64      WDL_betoh(WDL_INT64      big64)    { return WDL_be64toh(big64); }

	static WDL_ENDIAN_INLINE float          WDL_betof(unsigned int   big32)    { return WDL_be32tof(big32); }
	static WDL_ENDIAN_INLINE double         WDL_betof(WDL_UINT64     big64)    { return WDL_be64tof(big64); }

#endif // __cplusplus


#endif // _WDL_ENDIAN_H_
