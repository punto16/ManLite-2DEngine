#include "RendererEM.h"

#include "EngineCore.h"
#include "WindowEM.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Defs.h"

RendererEM::RendererEM(EngineCore* parent) : EngineModule(parent)
{
	vsync = true;
}

RendererEM::~RendererEM()
{
}

bool RendererEM::Awake()
{
	bool ret = true;

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		LOG(LogType::LOG_ERROR, "RendererEM: Initialize GLEW failure");
		return false;
	}

	return ret;
}

bool RendererEM::Start()
{
	bool ret = true;

	if (!CompileShaders()) return false;

	// 2. Configurar buffers de geometría (existente)
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// 3. Crear FBO y textura (nuevo)
	//-----------------------------------------
	// Configurar framebuffer
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Textura de color
	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbSize.x, fbSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

	// Renderbuffer para depth/stencil
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbSize.x, fbSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		// Manejar error
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//-----------------------------------------

	// 4. Proyección (modificado para usar fbSize)
	engine->renderer_em->ResizeFBO(DEFAULT_CAM_WIDTH, DEFAULT_CAM_HEIGHT);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		LOG(LogType::LOG_ERROR, "Framebuffer error: {}", status);
		return false;
	}

	return ret;
}

bool RendererEM::PreUpdate()
{
	bool ret = true;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, fbSize.x, fbSize.y);

	// Limpiar buffers (existente)
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return ret;
}

bool RendererEM::Update(double dt)
{
	bool ret = true;

	glUseProgram(shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	RenderBatch();

	return ret;
}

bool RendererEM::PostUpdate()
{
	bool ret = true;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	int w, h;
	SDL_GetWindowSize(engine->window_em->GetSDLWindow(), &w, &h);
	glViewport(0, 0, w, h);

	//esto de abajo comentado por que no me interesa ver el render de opengl en la window, sino en el panel de imgui
	//SDL_GL_SwapWindow(engine->window_em->GetSDLWindow());

	return ret;
}

bool RendererEM::CleanUp()
{
	bool ret = true;

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// Limpiar FBO (nuevo)
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &renderTexture);
	glDeleteRenderbuffers(1, &rbo);

	// Limpiar shaders
	glDeleteProgram(shaderProgram);

	return ret;
}

bool RendererEM::CompileShaders()
{
	bool ret = true;
	const char* vertexShaderSource = R"glsl(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec4 aColor;
            layout (location = 2) in vec2 aTexCoords;
            
            out vec4 ourColor;
            out vec2 TexCoords;
            
            uniform mat4 projection;
            
            void main() {
                gl_Position = projection * vec4(aPos, 1.0);
                ourColor = aColor;
                TexCoords = aTexCoords;
            }
        )glsl";

	const char* fragmentShaderSource = R"glsl(
            #version 330 core
            in vec4 ourColor;
            in vec2 TexCoords;
            
            out vec4 FragColor;
            
            uniform sampler2D texture1;
            
            void main() {
                FragColor = texture(texture1, TexCoords) * ourColor;
            }
        )glsl";

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Check vertex shader errors
	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		LOG(LogType::LOG_ERROR, "Vertex shader compilation failed: {}", infoLog);
		return false;
	}

	// Fragment Shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Check fragment shader errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		LOG(LogType::LOG_ERROR, "Fragment shader compilation failed: {}", infoLog);
		return false;
	}

	// Shader Program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		LOG(LogType::LOG_ERROR, "Shader program linking failed: {}", infoLog);
		return false;
	}

	// Cleanup shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return ret;
}

void RendererEM::RenderBatch()
{
	float vertices[] = {
		// posiciones      // colores       // coord. textura
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f
	};

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, 1); // Asegúrate de bindear una textura válida
	//glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
}

void RendererEM::ResizeFBO(int width, int height) {
	fbSize = { width, height };

	// Actualizar textura
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	// Actualizar renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	// Actualizar proyección
	projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//
//void RendererEM::SetViewPort(const SDL_Rect& rect)
//{
//	SDL_RenderSetViewport(renderer, &rect);
//}
//
//void RendererEM::ResetViewPort()
//{
//	SDL_RenderSetViewport(renderer, viewport);
//}
//
//void RendererEM::SetCamera(const SDL_Rect& rect)
//{
//	camera->w = rect.w;
//	camera->h = rect.h;
//	camera->x = rect.x;
//	camera->y = rect.y;
//}
//
//void RendererEM::MoveCamera(const SDL_Rect& rect)
//{
//	camera->w += rect.w;
//	camera->h += rect.h;
//	camera->x += rect.x;
//	camera->y += rect.y;
//}
//
//void RendererEM::CameraZoom(float zoom)
//{
//	int originalCenterX = camera->x + camera->w / 2;
//	int originalCenterY = camera->y + camera->h / 2;
//
//	camera->w += DEFAULT_CAM_WIDTH * -zoom;
//	camera->h += DEFAULT_CAM_HEIGHT * -zoom;
//	if (camera->w < 442) camera->w = 442;
//	if (camera->h < 234) camera->h = 234;
//
//	camera->x = originalCenterX - (camera->w / 2);
//	camera->y = originalCenterY - (camera->h / 2);
//}
//
//void RendererEM::ResetCamera()
//{
//	ResetCameraPos();
//	ResetCameraZoom();
//}
//
//void RendererEM::ResetCameraPos()
//{
//	camera->x = (DEFAULT_CAM_WIDTH * 0.5);
//	camera->y = (DEFAULT_CAM_HEIGHT * 0.5);
//}
//
//void RendererEM::ResetCameraZoom()
//{
//	int originalCenterX = camera->x + camera->w / 2;
//	int originalCenterY = camera->y + camera->h / 2;
//
//	camera->w = DEFAULT_CAM_WIDTH;
//	camera->h = DEFAULT_CAM_HEIGHT;
//
//	camera->x = originalCenterX - (camera->w / 2);
//	camera->y = originalCenterY - (camera->h / 2);
//}
//
//bool RendererEM::DrawTexture(SDL_Texture* tex, int x, int y, bool useCamera, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY)
//{
//	bool ret = true;
//	SDL_Rect rect;
//	int screenWidth, screenHeight;
//
//	SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
//
//	if (useCamera)
//	{
//		float scaleX = (float)screenWidth / camera->w;
//		float scaleY = (float)screenHeight / camera->h;
//		int camCenterX = camera->x + camera->w / 2;
//		int camCenterY = camera->y + camera->h / 2;
//
//		rect.x = static_cast<int>((x - camCenterX) * scaleX + screenWidth / 2);
//		rect.y = static_cast<int>((y - camCenterY) * scaleY + screenHeight / 2);
//
//		if (section != NULL)
//		{
//			rect.w = static_cast<int>(section->w * scaleX);
//			rect.h = static_cast<int>(section->h * scaleY);
//		}
//		else
//		{
//			int texW, texH;
//			SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);
//			rect.w = static_cast<int>(texW * scaleX);
//			rect.h = static_cast<int>(texH * scaleY);
//		}
//
//		if (pivotX != INT_MAX && pivotY != INT_MAX)
//		{
//			pivotX = static_cast<int>(pivotX * scaleX);
//			pivotY = static_cast<int>(pivotY * scaleY);
//		}
//	}
//	else
//	{
//		rect.x = x;
//		rect.y = y;
//		if (section != NULL) {
//			rect.w = section->w;
//			rect.h = section->h;
//		}
//		else
//		{
//			SDL_QueryTexture(tex, NULL, NULL, &rect.w, &rect.h);
//		}
//	}
//
//	SDL_Point pivot = { pivotX, pivotY };
//	if (SDL_RenderCopyEx(renderer, tex, section, &rect, angle, &pivot, SDL_FLIP_NONE) != 0)
//	{
//		LOG(LogType::LOG_ERROR, "Error Rendering Texture: %s", SDL_GetError());
//		ret = false;
//	}
//
//	return ret;
//}
//
//bool RendererEM::DrawRectangle(const SDL_Rect& rect, SDL_Color c, bool filled, bool useCamera) const
//{
//	bool ret = true;
//
//	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
//	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
//
//	SDL_Rect rec(rect);
//
//	if (useCamera)
//	{
//		int screenWidth, screenHeight;
//		SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
//		float scaleX = (float)screenWidth / camera->w;
//		float scaleY = (float)screenHeight / camera->h;
//		int camCenterX = camera->x + camera->w / 2;
//		int camCenterY = camera->y + camera->h / 2;
//
//		rec.x = (int)((rec.x - camCenterX) * scaleX + screenWidth / 2);
//		rec.y = (int)((rec.y - camCenterY) * scaleY + screenHeight / 2);
//		rec.w = (int)(rec.w * scaleX);
//		rec.h = (int)(rec.h * scaleY);
//	}
//
//	int result = filled ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);
//	if (result != 0)
//	{
//		LOG(LogType::LOG_ERROR, "Error Rendering Rectangle: %s", SDL_GetError());
//		ret = false;
//	}
//
//	return ret;
//}
//
//bool RendererEM::DrawLine(int x1, int y1, int x2, int y2, SDL_Color c, bool useCamera) const
//{
//	bool ret = true;
//
//	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
//	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
//
//	if (useCamera)
//	{
//		int screenWidth, screenHeight;
//		SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
//		float scaleX = (float)screenWidth / camera->w;
//		float scaleY = (float)screenHeight / camera->h;
//		int camCenterX = camera->x + camera->w / 2;
//		int camCenterY = camera->y + camera->h / 2;
//
//		x1 = (int)((x1 - camCenterX) * scaleX + screenWidth / 2);
//		y1 = (int)((y1 - camCenterY) * scaleY + screenHeight / 2);
//		x2 = (int)((x2 - camCenterX) * scaleX + screenWidth / 2);
//		y2 = (int)((y2 - camCenterY) * scaleY + screenHeight / 2);
//	}
//
//	if (SDL_RenderDrawLine(renderer, x1, y1, x2, y2) != 0)
//	{
//		LOG(LogType::LOG_ERROR, "Error Rendering Line: %s", SDL_GetError());
//		ret = false;
//	}
//
//	return ret;
//}
//
//bool RendererEM::DrawCircle(int x, int y, int rad, SDL_Color c, bool useCamera) const
//{
//	bool ret = true;
//	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
//	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
//
//	int screenWidth, screenHeight;
//	SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
//
//	const int numPoints = 20;
//	SDL_Point points[numPoints];
//
//	for (int i = 0; i < numPoints; ++i)
//	{
//		double angle = 2 * M_PI * i / (numPoints - 1);
//		int pointX = x + static_cast<int>(rad * cos(angle));
//		int pointY = y + static_cast<int>(rad * sin(angle));
//
//		if (useCamera)
//		{
//			float scaleX = (float)screenWidth / camera->w;
//			float scaleY = (float)screenHeight / camera->h;
//			int camCenterX = camera->x + camera->w / 2;
//			int camCenterY = camera->y + camera->h / 2;
//
//			pointX = static_cast<int>((pointX - camCenterX) * scaleX + screenWidth / 2);
//			pointY = static_cast<int>((pointY - camCenterY) * scaleY + screenHeight / 2);
//		}
//
//		points[i].x = pointX;
//		points[i].y = pointY;
//	}
//
//	if (SDL_RenderDrawLines(renderer, points, numPoints) != 0)
//	{
//		LOG(LogType::LOG_ERROR, "Error Rendering Circle: %s", SDL_GetError());
//		ret = false;
//	}
//
//	return ret;
//}
//
//void RendererEM::DrawGrid(int spacing, SDL_Color c, bool useCamera) const
//{
//	int screenWidth, screenHeight;
//	SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
//
//	if (useCamera)
//	{
//		float scaleX = (float)screenWidth / camera->w;
//		float scaleY = (float)screenHeight / camera->h;
//		int camCenterX = camera->x + camera->w / 2;
//		int camCenterY = camera->y + camera->h / 2;
//
//		int visibleLeft = camCenterX - (screenWidth / (2 * scaleX));
//		int visibleRight = camCenterX + (screenWidth / (2 * scaleX));
//		int visibleTop = camCenterY - (screenHeight / (2 * scaleY));
//		int visibleBottom = camCenterY + (screenHeight / (2 * scaleY));
//
//		int startX = (visibleLeft / spacing) * spacing;
//		int endX = visibleRight + spacing;
//		for (int x = startX; x <= endX; x += spacing)
//		{
//			DrawLine(x, visibleTop, x, visibleBottom, c, true);
//		}
//
//		int startY = (visibleTop / spacing) * spacing;
//		int endY = visibleBottom + spacing;
//		for (int y = startY; y <= endY; y += spacing)
//		{
//			DrawLine(visibleLeft, y, visibleRight, y, c, true);
//		}
//	}
//	else
//	{
//		int startX = 0;
//		int endX = screenWidth;
//		int startY = 0;
//		int endY = screenHeight;
//
//		for (int x = startX; x <= endX; x += spacing)
//		{
//			DrawLine(x, startY, x, endY, c, false);
//		}
//
//		for (int y = startY; y <= endY; y += spacing)
//		{
//			DrawLine(startX, y, endX, y, c, false);
//		}
//	}
//}