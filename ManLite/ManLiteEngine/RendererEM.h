#ifndef __RENDERER_EM_H__
#define __RENDERER_EM_H__
#pragma once

#include "EngineModule.h"
#include "SDL2/SDL.h"

class RendererEM : public EngineModule
{
public:
	RendererEM(EngineCore* parent);
	~RendererEM();

	bool Awake();
	bool Start();
	bool PreUpdate();
	bool Update(double dt);
	bool PostUpdate();
	bool CleanUp();

	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();

	void SetCamera(const SDL_Rect& rect);
	void MoveCamera(const SDL_Rect& rect);
	void ResetCamera();
	SDL_Rect* GetCamera() const { return camera; }

	void SetBackgroundColor(SDL_Color c) { this->background_color = c; }
	SDL_Color GetBackGroundColor() const { return this->background_color; }

	bool DrawTexture(SDL_Texture* tex, int x, int y, const SDL_Rect* section = NULL, float speed = 1.0f, double angle = 0, int pivotX = INT_MAX, int pivotY = INT_MAX);
	bool DrawRectangle(const SDL_Rect& rect, SDL_Color c, bool filled = true, bool useCamera = true) const;
	bool DrawLine(int x1, int y1, int x2, int y2, SDL_Color c, bool useCamera = true) const;
	bool DrawCircle(int x, int y, int rad, SDL_Color c, bool useCamera = true) const;

	SDL_Renderer* GetRenderer() const { return this->renderer; }

private:

	bool vsync;

	SDL_Renderer* renderer;
	SDL_Rect* camera;
	SDL_Rect* viewport;
	SDL_Color background_color;

};

#endif // !__EWINDOW_EM_H__