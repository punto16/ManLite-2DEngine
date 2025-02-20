#include "RendererEM.h"

#include "EngineCore.h"
#include "WindowEM.h"

RendererEM::RendererEM(EngineCore* parent) : EngineModule(parent)
{
	background_color = { 0,0,0,0 };
	vsync = true;
}

RendererEM::~RendererEM()
{
	delete renderer;
	delete camera;
	delete viewport;
}

bool RendererEM::Awake()
{
	bool ret = true;

	Uint32 flags = SDL_RENDERER_ACCELERATED;
	if (vsync) flags |= SDL_RENDERER_PRESENTVSYNC;
	renderer = SDL_CreateRenderer(engine->window_em->GetSDLWindow(), -1, flags);

	if (renderer == NULL) return false;
	else
	{
		camera = new SDL_Rect();
		camera->w = engine->window_em->GetSDLSurface()->w;
		camera->h = engine->window_em->GetSDLSurface()->h;
		camera->x = 0;
		camera->y = 0;
	}

	return ret;
}

bool RendererEM::Start()
{
	bool ret = true;

	SDL_RenderGetViewport(renderer, viewport);

	return ret;
}

bool RendererEM::PreUpdate()
{
	bool ret = true;

	SDL_RenderClear(renderer);

	return ret;
}

bool RendererEM::Update(double dt)
{
	bool ret = true;

	return ret;
}

bool RendererEM::PostUpdate()
{
	bool ret = true;

	SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
	SDL_RenderPresent(renderer);

	return ret;
}

bool RendererEM::CleanUp()
{
	bool ret = true;

	SDL_DestroyRenderer(renderer);
	renderer = nullptr;

	return ret;
}

void RendererEM::SetViewPort(const SDL_Rect& rect)
{
	SDL_RenderSetViewport(renderer, &rect);
}

void RendererEM::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, viewport);
}

void RendererEM::SetCamera(const SDL_Rect& rect)
{
	camera->w = rect.w;
	camera->h = rect.h;
	camera->x = rect.x;
	camera->y = rect.y;
}

void RendererEM::MoveCamera(const SDL_Rect& rect)
{
	camera->w += rect.w;
	camera->h += rect.h;
	camera->x += rect.x;
	camera->y += rect.y;
}

void RendererEM::ResetCamera()
{
	camera->w = engine->window_em->GetSDLSurface()->w;
	camera->h = engine->window_em->GetSDLSurface()->h;
	camera->x = 0;
	camera->y = 0;
}

bool RendererEM::DrawTexture(SDL_Texture* tex, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY)
{
	bool ret = true;

	SDL_Rect rect;
	rect.x = (int)(camera->x * speed) + x;
	rect.y = (int)(camera->y * speed) + y;

	if (section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else SDL_QueryTexture(tex, NULL, NULL, &rect.w, &rect.h);

	SDL_Point* p = new SDL_Point();

	if (pivotX != INT_MAX && pivotY != INT_MAX)
	{
		p->x = pivotX;
		p->y = pivotY;
	}

	if (SDL_RenderCopyEx(renderer, tex, section, &rect, angle, p, SDL_FLIP_NONE)) { return false; }

	return ret;
}

bool RendererEM::DrawRectangle(const SDL_Rect& rect, SDL_Color c, bool filled, bool useCamera) const
{
	bool ret = true;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

	SDL_Rect rec(rect);

	if (useCamera)
	{
		rec.x = (int)(camera->x + rect.x);
		rec.y = (int)(camera->y + rect.y);
	}

	int result = filled ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

	if (result != 0) return false;

	return ret;
}

bool RendererEM::DrawLine(int x1, int y1, int x2, int y2, SDL_Color c, bool useCamera) const
{
	bool ret = true;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

	int result = -1;

	if (useCamera) result = SDL_RenderDrawLine(renderer, camera->x + x1, camera->y + y1, camera->x + x1, camera->y + y1);
	else result = SDL_RenderDrawLine(renderer, x1, y1, x1, y1);

	if (result != 0) return false;

	return ret;
}

bool RendererEM::DrawCircle(int x, int y, int rad, SDL_Color c, bool useCamera) const
{
	bool ret = true;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

	int result = -1;
	const int numPoints = 20;
	SDL_Point points[numPoints];

	for (int i = 0; i < numPoints; ++i) {
		double angle = 2 * M_PI * i / (numPoints - 1);
		points[i].x = x + static_cast<int>(rad * cos(angle));
		points[i].y = y + static_cast<int>(rad * sin(angle));
	}

	result = SDL_RenderDrawLines(renderer, points, numPoints);

	if (result != 0) return false;

	return ret;
}
