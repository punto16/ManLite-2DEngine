#include "RendererEM.h"

#include "EngineCore.h"
#include "WindowEM.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Defs.h"

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

	this->scene_camera.SetZoom(100.0f);

	return ret;
}

bool RendererEM::PreUpdate()
{
	bool ret = true;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, fbSize.x, fbSize.y);

	// Limpiar buffers (existente)
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
			layout (location = 0) in vec2 aPos;
			uniform mat4 uViewProj;
			
			void main() {
			    gl_Position = uViewProj * vec4(aPos, 0.0, 1.0);
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
}

void RendererEM::ResizeFBO(int width, int height) {
	fbSize = { width, height };
	
	scene_camera.Resize(width, height);

	// Actualizar textura
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	// Actualizar renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Grid::Grid(float size, int divisions) {
	stepSize = size / divisions;  // Calculamos el tamaño entre divisiones

	// Geometría del quad de pantalla completa
	std::vector<float> vertices = {
		-1.0f, -1.0f,  // Esquina inferior izquierda
		 1.0f, -1.0f,  // Esquina inferior derecha
		-1.0f,  1.0f,  // Esquina superior izquierda
		 1.0f,  1.0f   // Esquina superior derecha
	};

	// Configuración del VAO/VBO
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Compilación de shaders
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

void Grid::Draw(const glm::mat4& viewProjMatrix) {
	glUseProgram(shaderProgram);

	// Matriz inversa para obtener coordenadas del mundo
	glm::mat4 viewProjInv = glm::inverse(viewProjMatrix);
	glUniformMatrix4fv(
		glGetUniformLocation(shaderProgram, "uViewProjInv"),
		1,
		GL_FALSE,
		glm::value_ptr(viewProjInv)
	);

	// Obtener parámetros de la cámara (ajustar según tu implementación)
	float zoom = engine->renderer_em->GetSceneCamera().GetZoom();  // Reemplazar con tu sistema de cámara
	glUniform1f(glGetUniformLocation(shaderProgram, "uZoom"), zoom);
	glUniform1f(glGetUniformLocation(shaderProgram, "uStepSize"), stepSize);

	// Dibujar el quad
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}