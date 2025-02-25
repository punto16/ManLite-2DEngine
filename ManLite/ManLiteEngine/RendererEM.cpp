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
		SDL_PIXELFORMAT_ABGR8888,
		SDL_TEXTUREACCESS_STREAMING,
		DEFAULT_CAM_WIDTH,
		DEFAULT_CAM_HEIGHT
	);
	if (renderer_texture == NULL) return false;

	renderer_target = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ABGR8888,
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

	SDL_Texture* prev_target = SDL_GetRenderTarget(renderer);
	void* pixels;
	int pitch;
	SDL_LockTexture(renderer_texture, nullptr, &pixels, &pitch);
	SDL_RenderReadPixels(
		renderer,
		nullptr,
		SDL_PIXELFORMAT_ABGR8888,
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
	int originalCenterX = camera->x + camera->w / 2;
	int originalCenterY = camera->y + camera->h / 2;

	camera->w += DEFAULT_CAM_WIDTH * -zoom;
	camera->h += DEFAULT_CAM_HEIGHT * -zoom;
	if (camera->w < 442) camera->w = 442;
	if (camera->h < 234) camera->h = 234;

	camera->x = originalCenterX - (camera->w / 2);
	camera->y = originalCenterY - (camera->h / 2);
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
	int originalCenterX = camera->x + camera->w / 2;
	int originalCenterY = camera->y + camera->h / 2;

	camera->w = DEFAULT_CAM_WIDTH;
	camera->h = DEFAULT_CAM_HEIGHT;

	camera->x = originalCenterX - (camera->w / 2);
	camera->y = originalCenterY - (camera->h / 2);
}

bool RendererEM::DrawTexture(SDL_Texture* tex, int x, int y, bool useCamera, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY)
{
	bool ret = true;
	SDL_Rect rect;
	int screenWidth, screenHeight;

	SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

	if (useCamera)
	{
		float scaleX = (float)screenWidth / camera->w;
		float scaleY = (float)screenHeight / camera->h;
		int camCenterX = camera->x + camera->w / 2;
		int camCenterY = camera->y + camera->h / 2;

		rect.x = static_cast<int>((x - camCenterX) * scaleX + screenWidth / 2);
		rect.y = static_cast<int>((y - camCenterY) * scaleY + screenHeight / 2);

		if (section != NULL)
		{
			rect.w = static_cast<int>(section->w * scaleX);
			rect.h = static_cast<int>(section->h * scaleY);
		}
		else
		{
			int texW, texH;
			SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);
			rect.w = static_cast<int>(texW * scaleX);
			rect.h = static_cast<int>(texH * scaleY);
		}

		if (pivotX != INT_MAX && pivotY != INT_MAX)
		{
			pivotX = static_cast<int>(pivotX * scaleX);
			pivotY = static_cast<int>(pivotY * scaleY);
		}
	}
	else
	{
		rect.x = x;
		rect.y = y;
		if (section != NULL) {
			rect.w = section->w;
			rect.h = section->h;
		}
		else
		{
			SDL_QueryTexture(tex, NULL, NULL, &rect.w, &rect.h);
		}
	}

	SDL_Point pivot = { pivotX, pivotY };
	if (SDL_RenderCopyEx(renderer, tex, section, &rect, angle, &pivot, SDL_FLIP_NONE) != 0)
	{
		LOG(LogType::LOG_ERROR, "Error Rendering Texture: %s", SDL_GetError());
		ret = false;
	}

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
		int screenWidth, screenHeight;
		SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
		float scaleX = (float)screenWidth / camera->w;
		float scaleY = (float)screenHeight / camera->h;
		int camCenterX = camera->x + camera->w / 2;
		int camCenterY = camera->y + camera->h / 2;

		rec.x = (int)((rec.x - camCenterX) * scaleX + screenWidth / 2);
		rec.y = (int)((rec.y - camCenterY) * scaleY + screenHeight / 2);
		rec.w = (int)(rec.w * scaleX);
		rec.h = (int)(rec.h * scaleY);
	}

	int result = filled ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);
	if (result != 0)
	{
		LOG(LogType::LOG_ERROR, "Error Rendering Rectangle: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool RendererEM::DrawLine(int x1, int y1, int x2, int y2, SDL_Color c, bool useCamera) const
{
	bool ret = true;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

	if (useCamera)
	{
		int screenWidth, screenHeight;
		SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
		float scaleX = (float)screenWidth / camera->w;
		float scaleY = (float)screenHeight / camera->h;
		int camCenterX = camera->x + camera->w / 2;
		int camCenterY = camera->y + camera->h / 2;

		x1 = (int)((x1 - camCenterX) * scaleX + screenWidth / 2);
		y1 = (int)((y1 - camCenterY) * scaleY + screenHeight / 2);
		x2 = (int)((x2 - camCenterX) * scaleX + screenWidth / 2);
		y2 = (int)((y2 - camCenterY) * scaleY + screenHeight / 2);
	}

	if (SDL_RenderDrawLine(renderer, x1, y1, x2, y2) != 0)
	{
		LOG(LogType::LOG_ERROR, "Error Rendering Line: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool RendererEM::DrawCircle(int x, int y, int rad, SDL_Color c, bool useCamera) const
{
	bool ret = true;
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

	int screenWidth, screenHeight;
	SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

	const int numPoints = 20;
	SDL_Point points[numPoints];

	for (int i = 0; i < numPoints; ++i)
	{
		double angle = 2 * M_PI * i / (numPoints - 1);
		int pointX = x + static_cast<int>(rad * cos(angle));
		int pointY = y + static_cast<int>(rad * sin(angle));

		if (useCamera)
		{
			float scaleX = (float)screenWidth / camera->w;
			float scaleY = (float)screenHeight / camera->h;
			int camCenterX = camera->x + camera->w / 2;
			int camCenterY = camera->y + camera->h / 2;

			pointX = static_cast<int>((pointX - camCenterX) * scaleX + screenWidth / 2);
			pointY = static_cast<int>((pointY - camCenterY) * scaleY + screenHeight / 2);
		}

		points[i].x = pointX;
		points[i].y = pointY;
	}

	if (SDL_RenderDrawLines(renderer, points, numPoints) != 0)
	{
		LOG(LogType::LOG_ERROR, "Error Rendering Circle: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

void RendererEM::DrawGrid(int spacing, SDL_Color c, bool useCamera) const
{
	int screenWidth, screenHeight;
	SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

	if (useCamera)
	{
		float scaleX = (float)screenWidth / camera->w;
		float scaleY = (float)screenHeight / camera->h;
		int camCenterX = camera->x + camera->w / 2;
		int camCenterY = camera->y + camera->h / 2;

		int visibleLeft = camCenterX - (screenWidth / (2 * scaleX));
		int visibleRight = camCenterX + (screenWidth / (2 * scaleX));
		int visibleTop = camCenterY - (screenHeight / (2 * scaleY));
		int visibleBottom = camCenterY + (screenHeight / (2 * scaleY));

		int startX = (visibleLeft / spacing) * spacing;
		int endX = visibleRight + spacing;
		for (int x = startX; x <= endX; x += spacing)
		{
			DrawLine(x, visibleTop, x, visibleBottom, c, true);
		}

		int startY = (visibleTop / spacing) * spacing;
		int endY = visibleBottom + spacing;
		for (int y = startY; y <= endY; y += spacing)
		{
			DrawLine(visibleLeft, y, visibleRight, y, c, true);
		}
	}
	else
	{
		int startX = 0;
		int endX = screenWidth;
		int startY = 0;
		int endY = screenHeight;

		for (int x = startX; x <= endX; x += spacing)
		{
			DrawLine(x, startY, x, endY, c, false);
		}

		for (int y = startY; y <= endY; y += spacing)
		{
			DrawLine(startX, y, endX, y, c, false);
		}
	}
}