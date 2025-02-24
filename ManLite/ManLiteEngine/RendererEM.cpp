#include "RendererEM.h"

#include "EngineCore.h"
#include "WindowEM.h"

RendererEM::RendererEM(EngineCore* parent) : EngineModule(parent)
{
	background_color.r = 200;
	background_color.g = 200;
	background_color.b = 200;
	background_color.a = 255;
	vsync = true;
}

RendererEM::~RendererEM()
{
	delete renderer;
	delete renderer_texture;
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
		viewport = new SDL_Rect();
		camera = new SDL_Rect();
		ResetCamera();
	}

	renderer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		DEFAULT_CAM_WIDTH,
		DEFAULT_CAM_HEIGHT
	);
	if (renderer_texture == NULL) return false;

	renderer_target = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		DEFAULT_CAM_WIDTH,
		DEFAULT_CAM_HEIGHT
	);
	if (renderer_target == NULL) return false;

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

	SDL_SetRenderTarget(renderer, renderer_target);
	SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
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

	SDL_Rect r = { -5,-5,10,10 };
	DrawRectangle(r, { 0,0,0,255 }, true);
	DrawGrid(50,50, { 0,0,0,100 });

	SDL_Texture* prev_target = SDL_GetRenderTarget(renderer);
	void* pixels;
	int pitch;
	SDL_LockTexture(renderer_texture, nullptr, &pixels, &pitch);
	SDL_RenderReadPixels(
		renderer,
		nullptr,
		SDL_PIXELFORMAT_RGBA8888,
		pixels,
		pitch
	);
	SDL_UnlockTexture(renderer_texture);
	SDL_SetRenderTarget(renderer, prev_target);
	SDL_UpdateTexture(renderer_texture, nullptr, pixels, DEFAULT_CAM_WIDTH * 4);

	SDL_SetRenderTarget(renderer, NULL);

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

void RendererEM::CameraZoom(float zoom)
{

}

void RendererEM::ResetCamera()
{
	ResetCameraPos();
	ResetCameraZoom();
}

void RendererEM::ResetCameraPos()
{
	camera->x = (DEFAULT_CAM_WIDTH * 0.5);
	camera->y = (DEFAULT_CAM_HEIGHT * 0.5);
}

void RendererEM::ResetCameraZoom()
{
	camera->w = DEFAULT_CAM_WIDTH;
	camera->h = DEFAULT_CAM_HEIGHT;
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

	if (useCamera) result = SDL_RenderDrawLine(renderer, camera->x + x1, camera->y + y1, camera->x + x2, camera->y + y2);
	else result = SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

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

void RendererEM::DrawGrid(int rows, int columns, SDL_Color c, bool useCamera) const {
	const int spacing = 100;

	const int halfWidth = (columns * spacing) / 2;
	const int halfHeight = (rows * spacing) / 2;

	for (int i = 0; i <= rows; ++i) {
		int y = i * spacing - halfHeight;
		DrawLine(-halfWidth, y, halfWidth, y, c, useCamera);
	}

	for (int i = 0; i <= columns; ++i) {
		int x = i * spacing - halfWidth;
		DrawLine(x, -halfHeight, x, halfHeight, c, useCamera);
	}
}
