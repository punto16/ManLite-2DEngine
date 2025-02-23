#include "PanelScene.h"

#include "GUI.h"
#include "EngineCore.h"
#include "RendererEM.h"

#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>
#include <gl/GL.h>
#include <imgui.h>
#include <vector>

PanelScene::PanelScene(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelScene::~PanelScene()
{
}

void PanelScene::Start()
{
	glGenTextures(1, &openglTextureID);
	glBindTexture(GL_TEXTURE_2D, openglTextureID);
	std::vector<uint8_t> emptyData(1700 * 900 * 4, 0); // RGBA: 0,0,0,0
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1700, 900, 0, GL_RGBA, GL_UNSIGNED_BYTE, emptyData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool PanelScene::Update()
{
	bool ret = true;

	SDL_Texture* sdlTexture = engine->renderer_em->GetRendererTexture();
	void* pixels;
	int pitch;
	SDL_LockTexture(sdlTexture, nullptr, &pixels, &pitch);

	glEnable(GL_TEXTURE_2D);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1700, 900, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	SDL_UnlockTexture(sdlTexture);


	if (ImGui::Begin(name.c_str(), &enabled))
	{
		ImGui::Image(
			(ImTextureID)(intptr_t)openglTextureID,
			ImVec2(1700, 900),
			ImVec2(0, 0),
			ImVec2(1, 1)
		);
	}
	ImGui::End();

	return ret;
}

bool PanelScene::CleanUp()
{
	bool ret = true;

	if (openglTextureID != 0)
	{
		glDeleteTextures(1, &openglTextureID);
		openglTextureID = 0;
	}

	return ret;
}
