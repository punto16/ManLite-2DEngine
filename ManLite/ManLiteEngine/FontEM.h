#ifndef __FONT_EM_H__
#define __FONT_EM_H__
#pragma once

#include "EngineModule.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class FontEM : public EngineModule
{
public:
	FontEM(EngineCore* parent);
	virtual ~FontEM();

	bool Awake();
	bool Start();

	bool CleanUp();

	FT_Library GetFreeType() const { return ftLibrary; }

private:
	FT_Library ftLibrary;
};

#endif // !__FONT_EM_H__