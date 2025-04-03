#include "FontEM.h"

#include "EngineCore.h"
#include "Defs.h"

#include <SDL2/SDL_image.h>

FontEM::FontEM(EngineCore* parent) :
	EngineModule(parent),
	ftLibrary(nullptr)
{

}

FontEM::~FontEM()
{
}

bool FontEM::Awake()
{
    FT_Error error = FT_Init_FreeType(&ftLibrary);
    if (FT_Init_FreeType(&ftLibrary))
    {
        LOG(LogType::LOG_ERROR, "FontEM: Could not init FreeType Library. Error code: {}", error);
        return false;
    }

    LOG(LogType::LOG_OK, "FontEM: FreeType initialized successfully");
    return true;
}

bool FontEM::Start()
{
	bool ret = true;

	return ret;
}

bool FontEM::CleanUp()
{
	bool ret = true;

    if (ftLibrary)
    {
        FT_Done_FreeType(ftLibrary);
        ftLibrary = nullptr;
        LOG(LogType::LOG_OK, "FontEM: FreeType resources cleaned");
    }

	return ret;
}