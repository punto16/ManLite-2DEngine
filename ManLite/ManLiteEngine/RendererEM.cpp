#include "RendererEM.h"

#include "EngineCore.h"
#include "WindowEM.h"
#include "Camera.h"
#include "GameObject.h"
#include "SceneManagerEM.h"
#include "ResourceManager.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Defs.h"
#include "mat3f.h"

RendererEM::RendererEM(EngineCore* parent) : EngineModule(parent), scene_camera(DEFAULT_CAM_WIDTH, DEFAULT_CAM_HEIGHT)
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

	LOG(LogType::LOG_OK, "RendererEM: Initialization Success");

	return ret;
}

bool RendererEM::Start()
{
	bool ret = true;

	if (!CompileShaders()) return false;

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

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	if (engine->GetEditorOrBuild())
	{
		glGenTextures(1, &renderTexture);
		glBindTexture(GL_TEXTURE_2D, renderTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbSize.x, fbSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);
	}

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbSize.x, fbSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG(LogType::LOG_ERROR, "RendererEM: Frame Buffer failure");
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	engine->renderer_em->ResizeFBO(DEFAULT_CAM_WIDTH, DEFAULT_CAM_HEIGHT);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		LOG(LogType::LOG_ERROR, "Framebuffer error: {}", status);
		return false;
	}

	glGenSamplers(1, &samplerLinear);
	glGenSamplers(1, &samplerNearest);

	// sampler for NON pixel art
	glSamplerParameteri(samplerLinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(samplerLinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(samplerLinear, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(samplerLinear, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// samper for pixel art
	glSamplerParameteri(samplerNearest, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(samplerNearest, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(samplerNearest, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(samplerNearest, GL_TEXTURE_WRAP_T, GL_REPEAT);

	this->scene_camera.SetZoom(100.0f);
	SetupQuad();

	GLuint tVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(tVertexShader, 1, &textVertexShader, NULL);
	glCompileShader(tVertexShader);

	GLuint tFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(tFragmentShader, 1, &textFragmentShader, NULL);
	glCompileShader(tFragmentShader);

	textShaderProgram = glCreateProgram();
	glAttachShader(textShaderProgram, tVertexShader);
	glAttachShader(textShaderProgram, tFragmentShader);
	glLinkProgram(textShaderProgram);

	glDeleteShader(tVertexShader);
	glDeleteShader(tFragmentShader);

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, &debugVertexShader, NULL);
	glCompileShader(vShader);

	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, &debugFragmentShader, NULL);
	glCompileShader(fShader);

	debugShaderProgram = glCreateProgram();
	glAttachShader(debugShaderProgram, vShader);
	glAttachShader(debugShaderProgram, fShader);
	glLinkProgram(debugShaderProgram);

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	float lineVertices[] = {
		-0.5f,  0.5f, // Rectangle
		 0.5f,  0.5f,
		 0.5f, -0.5f,
		-0.5f, -0.5f,
		-0.5f,  0.5f
	};

	glGenVertexArrays(1, &lineVAO);
	glGenBuffers(1, &lineVBO);

	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	SetupDebugShapes();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	return ret;
}

bool RendererEM::PreUpdate()
{
	bool ret = true;

	if (engine->GetEditorOrBuild())
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, fbSize.x, fbSize.y);
	}
	else
	{
		unsigned int w, h;
		engine->window_em->GetWindowSize(w, h);
		if (w != fbSize.x || h != fbSize.y)
		{
			ResizeFBO(w, h);
		}
		glViewport(0, 0, w, h);
	}
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return ret;
}

bool RendererEM::Update(double dt)
{
	bool ret = true;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	RenderBatch();
	RenderDebugColliders();

	glDisable(GL_BLEND);

	return ret;
}

bool RendererEM::PostUpdate()
{
	bool ret = true;

	if (engine->GetEditorOrBuild())
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		int w, h;
		SDL_GetWindowSize(engine->window_em->GetSDLWindow(), &w, &h);
		glViewport(0, 0, w, h);
	}
	else
	{
		SDL_GL_SwapWindow(engine->window_em->GetSDLWindow());
	}

	return ret;
}

bool RendererEM::CleanUp()
{
	bool ret = true;

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteBuffers(1, &quadEBO);

	glDeleteFramebuffers(1, &fbo);
	if (engine->GetEditorOrBuild())
		glDeleteTextures(1, &renderTexture);
	glDeleteRenderbuffers(1, &rbo);

	glDeleteProgram(shaderProgram);

	return ret;
}

bool RendererEM::CompileShaders()
{
	bool ret = true;
	const char* vertexShaderSource = R"glsl(
			#version 330 core
			layout (location = 0) in vec2 aPos;
			layout (location = 1) in vec2 aTexCoords;
			
			// Atributos instanciados
			layout (location = 2) in mat4 instanceModel;
			layout (location = 6) in vec4 instanceUVRect;
			
			uniform mat4 uViewProj;
			
			out vec2 TexCoords;
			
			void main() {
			    gl_Position = uViewProj * instanceModel * vec4(aPos, 0.0, 1.0);
			    vec2 uvOffset = instanceUVRect.xy;
			    vec2 uvScale = instanceUVRect.zw - instanceUVRect.xy;
			    TexCoords = uvOffset + aTexCoords * uvScale;
			})glsl";

	const char* fragmentShaderSource = R"glsl(
			#version 330 core
			in vec2 TexCoords;
			out vec4 FragColor;
			
			uniform sampler2D uTexture;
			
			void main() {
			    FragColor = texture(uTexture, TexCoords);
			    
			    if (FragColor.a < 0.1)
			        discard;
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
		LOG(LogType::LOG_ERROR, "RendererEM: Vertex shader compilation failed: {}", infoLog);
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
		LOG(LogType::LOG_ERROR, "RendererEM: Fragment shader compilation failed: {}", infoLog);
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
		LOG(LogType::LOG_ERROR, "RendererEM: Shader program linking failed: {}", infoLog);
		return false;
	}

	glGenBuffers(1, &instanceModelVBO);
	glGenBuffers(1, &instanceUVVBO);
	glGenBuffers(1, &instanceColorVBO);

	// Cleanup shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return ret;
}

void RendererEM::RenderBatch() {


	glm::mat4 viewProj;
	if (use_scene_cam && engine->GetEditorOrBuild()) {
		viewProj = scene_camera.GetViewProjMatrix();
	}
	else {
		GameObject* cam_go = &engine->scene_manager_em->GetCurrentScene().GetCurrentCameraGO();
		if (cam_go && cam_go->GetComponent<Camera>()) {
			auto camera = cam_go->GetComponent<Camera>();
			viewProj = camera->GetProjectionMatrix() * camera->GetViewMatrix();
		}
		else {
			viewProj = scene_camera.GetViewProjMatrix();
		}
	}


	// Ordenar sprites por textura, sampler y tipo
	std::sort(spritesToRender.begin(), spritesToRender.end(),
		[](const SpriteRenderData& a, const SpriteRenderData& b) {
			return std::tie(a.textureID, a.pixel_art, a.text, a.layerOrder) <
				std::tie(b.textureID, b.pixel_art, b.text, b.layerOrder);
		});

	// Procesar batches
	GLuint currentTexture = 0;
	bool currentPixelArt = false;
	bool currentIsText = false;
	std::vector<glm::mat4> models;
	std::vector<glm::vec4> uvRects;
	std::vector<glm::vec4> colors;

	auto flushBatch = [&]() {
		if (models.empty()) return;

		// Seleccionar shader
		GLuint program = currentIsText ? textShaderProgram : shaderProgram;
		glUseProgram(program);

		GLuint uViewProjLoc = glGetUniformLocation(program, "uViewProj");
		glUniformMatrix4fv(uViewProjLoc, 1, GL_FALSE, glm::value_ptr(viewProj));

		// Configurar textura y sampler
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		glBindSampler(0, currentPixelArt ? samplerNearest : samplerLinear);

		// Actualizar buffers de instancias
		glBindBuffer(GL_ARRAY_BUFFER, instanceModelVBO);
		glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), models.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, instanceUVVBO);
		glBufferData(GL_ARRAY_BUFFER, uvRects.size() * sizeof(glm::vec4), uvRects.data(), GL_DYNAMIC_DRAW);

		if (currentIsText) {
			glBindBuffer(GL_ARRAY_BUFFER, instanceColorVBO);
			glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_DRAW);
		}

		// Dibujar instancias
		glBindVertexArray(quadVAO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, models.size());

		// Resetear buffers
		models.clear();
		uvRects.clear();
		colors.clear();
		};

	for (const auto& sprite : spritesToRender) {
		if (sprite.textureID != currentTexture ||
			sprite.pixel_art != currentPixelArt ||
			sprite.text != currentIsText)
		{
			flushBatch();
			currentTexture = sprite.textureID;
			currentPixelArt = sprite.pixel_art;
			currentIsText = sprite.text;
		}

		models.push_back(sprite.modelMatrix);
		uvRects.emplace_back(sprite.u1, sprite.v1, sprite.u2, sprite.v2);

		if (currentIsText) {
			colors.push_back(sprite.color);
		}
	}

	flushBatch();
	spritesToRender.clear();
}

void RendererEM::ResizeFBO(int width, int height)
{
	fbSize = { width, height };

	scene_camera.Resize(width, height);

	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RendererEM::UseSceneViewCam()
{
	this->use_scene_cam = true;
}

void RendererEM::UseGameViewCam()
{
	this->use_scene_cam = false;
}

void RendererEM::SetupQuad()
{
	float vertices[] = {
		// Position      // UVs
		-0.5f,  0.5f,     0.0f, 1.0f,
		 0.5f,  0.5f,     1.0f, 1.0f,
		 0.5f, -0.5f,     1.0f, 0.0f,
		-0.5f, -0.5f,     0.0f, 0.0f
	};

	unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glGenBuffers(1, &quadEBO);

	glBindVertexArray(quadVAO);

	// 1. Buffer de vértices base (posición y UVs)
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Atributo 0: Posición
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Atributo 1: UVs
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// 2. Buffer de elementos
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// 3. Buffer de instancias: Model Matrix (atributos 2-5)
	glGenBuffers(1, &instanceModelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceModelVBO);
	for (int i = 0; i < 4; ++i) {
		glEnableVertexAttribArray(2 + i);
		glVertexAttribPointer(
			2 + i,                      // Location 
			4,                          // Componentes (vec4)
			GL_FLOAT,
			GL_FALSE,
			sizeof(glm::mat4),          // Stride completo de la matriz
			(void*)(i * sizeof(glm::vec4)) // Offset por columna
		);
		glVertexAttribDivisor(2 + i, 1); // Instanciado
	}

	// 4. Buffer de UV Rect (atributo 6)
	glGenBuffers(1, &instanceUVVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceUVVBO);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
	glVertexAttribDivisor(6, 1);

	// 5. Buffer de Color (atributo 7 - solo texto)
	glGenBuffers(1, &instanceColorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceColorVBO);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
	glVertexAttribDivisor(7, 1);

	glBindVertexArray(0);
}

glm::mat4 RendererEM::ConvertMat3fToGlmMat4(const mat3f& mat, float z)
{
	glm::mat4 result(1.0f);

	// Columns 0 & 1
	result[0][0] = mat.m[0];
	result[0][1] = mat.m[1];
	result[1][0] = mat.m[3];
	result[1][1] = mat.m[4];

	// Traslation (column 2 in mat3f)
	result[3][0] = mat.m[6];
	result[3][1] = mat.m[7];
	result[3][2] = z;

	return result;
}

void RendererEM::SetupDebugShapes()
{
	// 1. Círculo relleno (TRIANGLE_FAN)
	const int FILLED_CIRCLE_SEGMENTS = 32;
	std::vector<float> filledCircleVertices;
	filledCircleVertices.push_back(0.0f); // Centro (x)
	filledCircleVertices.push_back(0.0f); // Centro (y)
	for (int i = 0; i <= FILLED_CIRCLE_SEGMENTS; ++i) {
		float angle = 2.0f * PI * i / FILLED_CIRCLE_SEGMENTS;
		filledCircleVertices.push_back(cos(angle)); // x
		filledCircleVertices.push_back(sin(angle)); // y
	}
	glGenVertexArrays(1, &filledCircleVAO);
	glGenBuffers(1, &filledCircleVBO);
	glBindVertexArray(filledCircleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, filledCircleVBO);
	glBufferData(GL_ARRAY_BUFFER, filledCircleVertices.size() * sizeof(float), filledCircleVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// 2. Círculo contorno (LINE_LOOP)
	const int OUTLINE_CIRCLE_SEGMENTS = 64;
	std::vector<float> outlineCircleVertices;
	for (int i = 0; i <= OUTLINE_CIRCLE_SEGMENTS; ++i) {
		float angle = 2.0f * PI * i / OUTLINE_CIRCLE_SEGMENTS;
		outlineCircleVertices.push_back(cos(angle)); // x
		outlineCircleVertices.push_back(sin(angle)); // y
	}
	glGenVertexArrays(1, &outlineCircleVAO);
	glGenBuffers(1, &outlineCircleVBO);
	glBindVertexArray(outlineCircleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, outlineCircleVBO);
	glBufferData(GL_ARRAY_BUFFER, outlineCircleVertices.size() * sizeof(float), outlineCircleVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// 3. Cuadrado relleno (TRIANGLES)
	float filledQuadVertices[] = {
		// Triángulo 1
		-0.5f,  0.5f, // V0
		 0.5f,  0.5f, // V1
		-0.5f, -0.5f, // V2
		// Triángulo 2
		 0.5f,  0.5f, // V1
		 0.5f, -0.5f, // V3
		-0.5f, -0.5f  // V2
	};
	glGenVertexArrays(1, &filledQuadVAO);
	glGenBuffers(1, &filledQuadVBO);
	glBindVertexArray(filledQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, filledQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(filledQuadVertices), filledQuadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// 4. Cuadrado contorno (LINE_LOOP)
	float outlineQuadVertices[] = {
		-0.5f,  0.5f, // V0
		 0.5f,  0.5f, // V1
		 0.5f, -0.5f, // V2
		-0.5f, -0.5f, // V3
		-0.5f,  0.5f  // V0 (cierra el loop)
	};
	glGenVertexArrays(1, &outlineQuadVAO);
	glGenBuffers(1, &outlineQuadVBO);
	glBindVertexArray(outlineQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, outlineQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(outlineQuadVertices), outlineQuadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glGenBuffers(1, &modelMatricesBuffer);
	glGenBuffers(1, &colorsBuffer);

	SetupInstancedAttributes(filledCircleVAO);
	SetupInstancedAttributes(outlineCircleVAO);
	SetupInstancedAttributes(filledQuadVAO);
	SetupInstancedAttributes(outlineQuadVAO);

	filledCircleVertexCount = filledCircleVertices.size() / 2;  // +2 por el centro y cierre
	outlineCircleVertexCount = outlineCircleVertices.size() / 2;
	filledQuadVertexCount = 6;  // 2 triángulos = 6 vértices
	outlineQuadVertexCount = 5; // 5 puntos en LINE_LOOP
}

void RendererEM::SetupInstancedAttributes(GLuint VAO)
{
	glBindVertexArray(VAO);

	// Model matrices (mat4 ocupa 4 locations)
	glBindBuffer(GL_ARRAY_BUFFER, modelMatricesBuffer);
	for (int i = 0; i < 4; ++i) {
		glEnableVertexAttribArray(1 + i);
		glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
		glVertexAttribDivisor(1 + i, 1); // 1 = instanciado
	}

	// Colores
	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
	glVertexAttribDivisor(5, 1);
}

void RendererEM::SubmitSprite(GLuint textureID, const mat3f& modelMatrix, float u1, float v1, float u2, float v2, bool pixel_art, int order_in_layer, int order_in_component)
{
	spritesToRender.push_back({
	textureID,
	ConvertMat3fToGlmMat4(modelMatrix, (float)(order_in_layer / 1000 + order_in_component / 1000)),
	u1, v1, u2, v2,
	pixel_art,
	{ 1.0f, 1.0f, 1.0f, 1.0f },
	false,
	(float)(order_in_layer / 1000 + order_in_component / 1000)
		});
}

void RendererEM::SubmitDebugCollider(const mat3f& modelMatrix, const ML_Color& color, bool isCircle, int order_in_layer, int order_in_component, float radius, bool filled)
{
	if (isCircle)
	{
		RenderShapeInfo i;
		i.mat = mat3f::CreateTransformMatrix(modelMatrix.GetTranslation(), modelMatrix.GetRotation(), { radius, radius });
		i.color = color;
		i.layer_order = (float)(order_in_layer / 1000 + order_in_component / 1000);
		if (filled)
			debugCollidersCircleFilled.emplace_back(i);
		else
			debugCollidersCircleContorn.emplace_back(i);
	}
	else
	{
		RenderShapeInfo i;
		i.mat = modelMatrix;
		i.color = color;
		i.layer_order = (float)(order_in_layer / 1000 + order_in_component / 1000);
		if (filled)
			debugCollidersRectFilled.emplace_back(i);
		else
			debugCollidersRectContorn.emplace_back(i);
	}
}

void RendererEM::RenderDebugColliders() 
{
	glm::mat4 viewProj;
	if (use_scene_cam && engine->GetEditorOrBuild()) {
		viewProj = scene_camera.GetViewProjMatrix();
	}
	else {
		GameObject* cam_go = &engine->scene_manager_em->GetCurrentScene().GetCurrentCameraGO();
		if (cam_go && cam_go->GetComponent<Camera>()) {
			auto camera = cam_go->GetComponent<Camera>();
			viewProj = camera->GetProjectionMatrix() * camera->GetViewMatrix();
		}
		else {
			viewProj = scene_camera.GetViewProjMatrix();
		}
	}

	glUseProgram(debugShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(debugShaderProgram, "uViewProj"), 1, GL_FALSE, glm::value_ptr(viewProj));

	if (!debugCollidersCircleFilled.empty())
	{
		RenderBatchShapes(debugCollidersCircleFilled, filledCircleVAO, GL_TRIANGLE_FAN, filledCircleVertexCount);
		debugCollidersCircleFilled.clear();
	}

	if (!debugCollidersCircleContorn.empty())
	{
		glLineWidth(2.0f);
		RenderBatchShapes(debugCollidersCircleContorn, outlineCircleVAO, GL_LINE_LOOP, outlineCircleVertexCount);
		debugCollidersCircleContorn.clear();
		glLineWidth(1.0f);
	}

	if (!debugCollidersRectFilled.empty())
	{
		RenderBatchShapes(debugCollidersRectFilled, filledQuadVAO, GL_TRIANGLES, filledQuadVertexCount);
		debugCollidersRectFilled.clear();
	}

	if (!debugCollidersRectContorn.empty())
	{
		glLineWidth(2.0f);
		RenderBatchShapes(debugCollidersRectContorn, outlineQuadVAO, GL_LINE_LOOP, outlineQuadVertexCount);
		debugCollidersRectContorn.clear();
		glLineWidth(1.0f);
	}
}

void RendererEM::SubmitText(std::string text, FontData* font, const mat3f& modelMatrix, const ML_Color& color, TextAlignment ta)
{
	if (font == nullptr) return;

	float cursorY = 0.0f;
	float lineHeight = font->face->size->metrics.height >> 6;
	std::istringstream iss(text);
	std::string line;

	while (std::getline(iss, line, '\n'))
	{
		float lineWidth = 0.0f;
		FT_UInt prevGlyph = 0;
		std::vector<FontCharacter*> lineCharacters;

		for (char c : line) {
			FontCharacter* ch = ResourceManager::GetInstance().LoadFontCharacter(font, c);
			if (!ch) continue;

			if (prevGlyph) {
				FT_Vector kerning;
				FT_Get_Kerning(font->face, prevGlyph, FT_Get_Char_Index(font->face, c),
					FT_KERNING_DEFAULT, &kerning);
				lineWidth += kerning.x >> 6;
			}
			prevGlyph = FT_Get_Char_Index(font->face, c);

			lineWidth += ch->advance >> 6;
			lineCharacters.push_back(ch);
		}

		float startX = 0.0f;
		switch (ta) {
		case TEXT_ALIGN_CENTER:
			startX = -lineWidth / 2.0f;
			break;
		case TEXT_ALIGN_RIGHT:
			startX = -lineWidth;
			break;
		default:
			break;
		}

		float cursorX = startX;
		prevGlyph = 0;

		for (FontCharacter* ch : lineCharacters)
		{
			if (prevGlyph) {
				FT_Vector kerning;
				FT_Get_Kerning(font->face, prevGlyph, FT_Get_Char_Index(font->face, ch->charCode),
					FT_KERNING_DEFAULT, &kerning);
				cursorX += kerning.x >> 6;
			}
			prevGlyph = FT_Get_Char_Index(font->face, ch->charCode);

			float xpos = cursorX + ch->bearing.x + (ch->size.x / 2);
			float ypos = cursorY - (ch->size.y - ch->bearing.y) + (ch->size.y / 2);

			mat3f new_mat = mat3f::CreateTransformMatrix(
				{ xpos, ypos },
				0,
				{ ch->size.x, ch->size.y }
			);

			mat3f charModel = modelMatrix * new_mat;

			spritesToRender.push_back({
				ch->textureID,
				ConvertMat3fToGlmMat4(charModel),
				0.0f, 1.0f,
				1.0f, 0.0f,
				true,
				{
					color.r / 255.0f,
					color.g / 255.0f,
					color.b / 255.0f,
					color.a / 255.0f
				},
				true
				});

			cursorX += ch->advance >> 6;
		}

		cursorY -= lineHeight;
	}
}

Grid::Grid(float size, int divisions)
{
	stepSize = size / divisions;

	std::vector<float> vertices = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f,  1.0f
	};

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &gridVertexShader, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &gridFragmentShader, NULL);
	glCompileShader(fragmentShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Grid::Draw(const glm::mat4& viewProjMatrix)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(shaderProgram);

	glm::mat4 viewProjInv = glm::inverse(viewProjMatrix);
	glUniformMatrix4fv(
		glGetUniformLocation(shaderProgram, "uViewProjInv"),
		1,
		GL_FALSE,
		glm::value_ptr(viewProjInv)
	);

	float zoom = engine->renderer_em->GetSceneCamera().GetZoom();
	glUniform1f(glGetUniformLocation(shaderProgram, "uZoom"), zoom);
	glUniform1f(glGetUniformLocation(shaderProgram, "uStepSize"), stepSize);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}