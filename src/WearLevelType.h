#ifndef _WEAR_LEVEL_TYPE_
#define _WEAR_LEVEL_TYPE_

#include <stdint.h>


/// <summary>
/// No Wear levelling option.
/// </summary>
enum class NoWearLevel : uint8_t
{
	x1 = 1
};

/// <summary>
/// Wear levelling tiny options.
/// A single block can count up to 9 with no erasures.
/// x1 option doesn't make use of rolling counter, use StorageUnit instead.
/// </summary>
enum class WearLevelTiny : uint8_t
{
	x2 = 2,
	x3 = 3,
	x4 = 4,
	x5 = 5,
	x6 = 6,
	x7 = 7,
	x8 = 8,
	x9 = 9
};


/// <summary>
/// Wear levelling short options.
/// 2 blocks can count up to 17 with no erasures.
/// For smaller options, use WearLevelUnit<WearLevelShort>.
/// </summary>
enum class WearLevelShort : uint8_t
{
	x10 = 10,
	x11 = 11,
	x12 = 12,
	x13 = 13,
	x14 = 14,
	x15 = 15,
	x16 = 16,
	x17 = 17
};


/// <summary>
/// Wear levelling long options.
/// 4 blocks can count up to 33 with no erasures.
/// For smaller options, use WearLevelUnit<WearLevelShort>.
/// </summary>
enum class WearLevelLong : uint8_t
{
	x18 = 18,
	x19 = 19,
	x20 = 20,
	x21 = 21,
	x22 = 22,
	x23 = 23,
	x24 = 24,
	x25 = 25,
	x26 = 26,
	x27 = 27,
	x28 = 28,
	x29 = 29,
	x30 = 30,
	x31 = 31,
	x32 = 32,
	x33 = 33
};

/// <summary>
/// Wear levelling long options.
/// 8 blocks can count up to 65 with no erasures.
/// For smaller options, use WearLevelUnit<WearLevelLong>.
/// </summary>
enum class WearLevelLongLong : uint8_t
{
	x34 = 34,
	x35 = 35,
	x36 = 36,
	x37 = 37,
	x38 = 38,
	x39 = 39,
	x40 = 40,
	x41 = 41,
	x42 = 42,
	x43 = 43,
	x44 = 44,
	x45 = 45,
	x46 = 46,
	x47 = 47,
	x48 = 48,
	x49 = 49,
	x50 = 50,
	x51 = 51,
	x52 = 52,
	x53 = 53,
	x54 = 54,
	x55 = 55,
	x56 = 56,
	x57 = 57,
	x58 = 58,
	x59 = 59,
	x60 = 60,
	x61 = 61,
	x62 = 62,
	x63 = 63,
	x64 = 64,
	x65 = 65
};

#endif