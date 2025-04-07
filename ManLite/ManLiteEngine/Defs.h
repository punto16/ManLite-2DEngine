#ifndef __DEFS_H__
#define __DEFS_H__
#pragma once

#include <glm/glm.hpp>
#include <stdio.h>
#include <memory>
#include <random>
#include "string"

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

using vec2 = glm::dvec2;
using vec3 = glm::dvec3;
using vec4 = glm::dvec4;
using mat4 = glm::dmat4;
using mat4f = glm::mat4;

using vec2f = glm::vec2;
using vec3f = glm::vec3;
using vec4f = glm::vec4;

using quat = glm::dquat;
using quatf = glm::quat;

// manlite selfdones classes
class ML_Rect
{
public:
	ML_Rect(int x, int y, int w, int h)
	{
		this->x = (float)x;
		this->y = (float)y;
		this->w = (float)w;
		this->h = (float)h;
	}
	ML_Rect(float x, float y, float w, float h)
	{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}
	ML_Rect()
	{
		this->x = 0;
		this->y = 0;
		this->w = 0;
		this->h = 0;
	}
	~ML_Rect()
	{}

	float x, y, w, h;
};

//meant for 0 to 255
class ML_Color
{
public:
	ML_Color(int r, int g, int b, int a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
	ML_Color(float r, float g, float b, float a)
	{
		this->r = (int)r;
		this->g = (int)g;
		this->b = (int)b;
		this->a = (int)a;
	}
	ML_Color()
	{
		this->r = 0;
		this->g = 0;
		this->b = 0;
		this->a = 0;
	}
	~ML_Color()
	{
	}

	int r, g, b, a;
};

inline std::mt19937& GetEngineRandom() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	return gen;
}

template<typename T>
T RandomRange(const T& min_val, const T& max_val) {
	static_assert(std::is_arithmetic<T>::value, "T must be numeric");

	static std::mt19937 gen = GetEngineRandom();

	if constexpr (std::is_integral<T>::value) {
		std::uniform_int_distribution<T> distrib(std::min(min_val, max_val),
			std::max(min_val, max_val));
		return distrib(gen);
	}
	else {
		std::uniform_real_distribution<T> distrib(std::min(min_val, max_val),
			std::max(min_val, max_val));
		return distrib(gen);
	}
}

inline vec2f RandomRange(const vec2f& min_val, const vec2f& max_val) {
	return vec2f(
		RandomRange(min_val.x, max_val.x),
		RandomRange(min_val.y, max_val.y)
	);
}

inline ML_Color RandomRange(const ML_Color& min_val, const ML_Color& max_val) {
	return ML_Color(
		RandomRange(min_val.r, max_val.r),
		RandomRange(min_val.g, max_val.g),
		RandomRange(min_val.b, max_val.b),
		RandomRange(min_val.a, max_val.a)
	);
}

inline std::string RandomCharacter(const std::string& text) {
	if (text.empty()) return "";

	size_t random_index = RandomRange<size_t>(0, text.size() - 1);

	return std::string(1, text[random_index]);
}

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
#define MAX_LOGS_CONSOLE 250
// -------------------------------------------------------------------------------

#endif	// !__DEFS_H__