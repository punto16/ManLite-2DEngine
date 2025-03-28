#include "RendererEM.h"

#include "EngineCore.h"
#include "WindowEM.h"
#include "Camera.h"
#include "GameObject.h"
#include "SceneManagerEM.h"

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

	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbSize.x, fbSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

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

	// Configurar sampler para modo lineal (suavizado)
	glSamplerParameteri(samplerLinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(samplerLinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(samplerLinear, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(samplerLinear, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Configurar sampler para pixel art (sin suavizado)
	glSamplerParameteri(samplerNearest, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(samplerNearest, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(samplerNearest, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(samplerNearest, GL_TEXTURE_WRAP_T, GL_REPEAT);

	this->scene_camera.SetZoom(100.0f);
	SetupQuad();

	return ret;
}

bool RendererEM::PreUpdate()
{
	bool ret = true;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, fbSize.x, fbSize.y);

	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return ret;
}

bool RendererEM::Update(double dt)
{
	bool ret = true;

	glUseProgram(shaderProgram);
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
			
			uniform mat4 uModel;
			uniform mat4 uViewProj;
			uniform vec4 uUVRect; // x = u1, y = v1, z = u2, w = v2
			
			out vec2 TexCoords;
			
			void main() {
			    gl_Position = uViewProj * uModel * vec4(aPos, 0.0, 1.0);
			    
			    // Calcula las coordenadas UV aplicando el recorte
			    vec2 uvOffset = vec2(uUVRect.x, uUVRect.y);
			    vec2 uvScale = vec2(uUVRect.z - uUVRect.x, uUVRect.w - uUVRect.y);
			    TexCoords = uvOffset + aTexCoords * uvScale;
			}
        )glsl";

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

	// Cleanup shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return ret;
}

void RendererEM::RenderBatch()
{
	glUseProgram(shaderProgram);
	glBindVertexArray(quadVAO);

	glm::mat4 viewProj;
	GLuint uViewProjLoc = glGetUniformLocation(shaderProgram, "uViewProj");
	GLuint uModelLoc = glGetUniformLocation(shaderProgram, "uModel");
	GLuint uTextureLoc = glGetUniformLocation(shaderProgram, "uTexture");

	if (use_scene_cam) {
		viewProj = scene_camera.GetViewProjMatrix();
	}
	else {
		GameObject* cam_go = &engine->scene_manager_em->GetCurrentScene().GetCurrentCameraGO();
		if (cam_go && cam_go->GetComponent<Camera>())
		{
			auto camera = cam_go->GetComponent<Camera>();
			viewProj = camera->GetProjectionMatrix() * camera->GetViewMatrix();
		}
		else
		{
			viewProj = scene_camera.GetViewProjMatrix();
		}
	}

	glUniformMatrix4fv(uViewProjLoc, 1, GL_FALSE, glm::value_ptr(viewProj));
	glUniform1i(uTextureLoc, 0); // Usamos texture unit 0

	GLuint uUVRectLoc = glGetUniformLocation(shaderProgram, "uUVRect");

	for (const auto& sprite : spritesToRender) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sprite.textureID);

		if (sprite.pixel_art) {
			glBindSampler(0, samplerNearest);
		}
		else {
			glBindSampler(0, samplerLinear);
		}

		glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(sprite.modelMatrix));
		glUniform4f(uUVRectLoc, sprite.u1, sprite.v1, sprite.u2, sprite.v2);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	spritesToRender.clear();
	glBindVertexArray(0);
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
		// Position	     // UVs
		-0.5f,  0.5f,     0.0f, 1.0f,
		 0.5f,  0.5f,     1.0f, 1.0f,
		 0.5f, -0.5f,     1.0f, 0.0f,
		-0.5f, -0.5f,     0.0f, 0.0f
	};

	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glGenBuffers(1, &quadEBO);

	glBindVertexArray(quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Posición
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// UVs
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

glm::mat4 RendererEM::ConvertMat3fToGlmMat4(const mat3f& mat)
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

	return result;
}

void RendererEM::SubmitSprite(GLuint textureID, const mat3f& modelMatrix, float u1, float v1, float u2, float v2, bool pixel_art)
{
	spritesToRender.push_back({
	textureID,
	ConvertMat3fToGlmMat4(modelMatrix),
	u1, v1, u2, v2,
	pixel_art
		});
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