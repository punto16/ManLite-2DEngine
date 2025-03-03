#ifndef __DEFS_H__
#define __DEFS_H__
#pragma once

#include <stdio.h>
#include <memory>

//  NULL just in case ----------------------

#ifdef NULL
#undef NULL
#endif
#define NULL  0

// Deletes a buffer
#define RELEASE( x ) \
	{						\
	if( x != NULL )		\
		{					  \
	  delete x;			\
	  x = NULL;			  \
		}					  \
	}

// Deletes an array of buffers
#define RELEASE_ARRAY( x ) \
	{							  \
	if( x != NULL )			  \
		{							\
	  delete[] x;				\
	  x = NULL;					\
		}							\
							  \
	}

// PREDEFINED & GENERAL VARIABLES ------------------------------------------------
/// Keep a value between 0.0f and 1.0f
#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

#define TO_BOOL( a )  ( (a != 0) ? true : false )


/// Math
#define PI		 3.1415926535897932384f
#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f

//GAME TO WORLD
#define PIXELS_PER_METER 32.0f
#define METER_PER_PIXEL 0.03125f

#define METERS_TO_PIXELS(m) ((int) floor(PIXELS_PER_METER * m))
#define PIXEL_TO_METERS(p)  ((float) METER_PER_PIXEL * p)

/// Convert degrees to radians.
template <class T>
inline T ToRadians(const T degrees)
{
	return DEGTORAD * degrees;
}

/// Convert radians to degrees.
template <class T>
inline T ToDegrees(const T radians)
{
	return RADTODEG * radians;
}

/// Useful helpers, InRange checker, min, max and clamp
#define MIN( a, b ) ( ((a) < (b)) ? (a) : (b) )
#define MAX( a, b ) ( ((a) > (b)) ? (a) : (b) )
#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
#define IN_RANGE( value, min, max ) ( ((value) >= (min) && (value) <= (max)) ? 1 : 0 )

/// Standard string size
#define SHORT_STR	32
#define MID_STR		255
#define HUGE_STR	8192


// TYPEDEFS ----------------------------------------------------------------------
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;



// OTHERS ------------------------------------------------------------------------

#define DEFAULT_CAM_WIDTH 1700
#define DEFAULT_CAM_HEIGHT 900

/// Joins a path and file
inline const char* const PATH(const char* folder, const char* file)
{
	static char path[MID_STR];
	sprintf_s(path, MID_STR, "%s/%s", folder, file);
	return path;
}

/// Swaps 2 values
template <class VALUE_TYPE> void SWAP(VALUE_TYPE& a, VALUE_TYPE& b)
{
	VALUE_TYPE tmp = a;
	a = b;
	b = tmp;
}

/// Checks if a weak_ptr has not been initialized
template <typename T>
bool IsUninitialized(std::weak_ptr<T> const& weak)
{
	using wt = std::weak_ptr<T>;
	return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
}

// CONFIGURATION -----------------------------------------------------------------
#define TITLE "ManLite 2D Engine"
#define VERSION "v1.0"
#define ORGANIZATION "punto16"

// Folder paths
#define ASSETS_PATH "Assets\\"
#define LIBRARY_PATH "Library\\"

//Logs
#define BUFFER_SIZE 4096
#define MAX_LOGS_CONSOLE 1000
// -------------------------------------------------------------------------------

#endif	// !__DEFS_H__