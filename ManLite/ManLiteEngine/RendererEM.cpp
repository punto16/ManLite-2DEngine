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

	glGenFramebuffers(1, &fbo_lights);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_lights);

	glGenTextures(1, &lightRenderTexture);
	glBindTexture(GL_TEXTURE_2D, lightRenderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbSize.x, fbSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightRenderTexture, 0);

	glGenRenderbuffers(1, &rboLightsDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboLightsDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbSize.x, fbSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboLightsDepth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG(LogType::LOG_ERROR, "FBO de luces incompleto");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

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

	glSamplerParameteri(samplerLinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(samplerLinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(samplerLinear, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(samplerLinear, GL_TEXTURE_WRAP_T, GL_REPEAT);

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


	const char* lightVS = R"glsl(
#version 330 core
layout (location = 0) in vec2 aPos; // Quad en coordenadas NDC (-1 a 1)

void main() {
    // Proyección ortográfica simple para cubrir toda la pantalla
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)glsl";
	const char* lightFS = R"glsl(
#version 330 core
out vec4 FragColor;

uniform vec2 uScreenSize;
uniform vec3 uAmbientLight;
uniform int uNumLights;
uniform sampler2D uMainTexture;
uniform mat4 uViewProj; // Asegurar que se pasa la matriz de la cámara
uniform float uZoom;          // Zoom actual de la cámara
uniform float uBaseCamWidth;

struct Light {
    vec2 position;    // En coordenadas del MUNDO
    vec2 endPos;      // En coordenadas del MUNDO (para RayLight)
    vec3 color;
    float intensity;
    float radius;     // En píxeles
    float startRadius;
    float endRadius;
    int type;
};

uniform Light uLights[32];

float smoothAttenuation(float dist, float radius) {
    return 1.0 - smoothstep(radius * 0.7, radius, dist);
}

float distanceToSegment(vec2 p, vec2 a, vec2 b) {
    vec2 ap = p - a;
    vec2 ab = b - a;
    float t = clamp(dot(ap, ab) / dot(ab, ab), 0.0, 1.0);
    return length(ap - ab * t);
}

vec2 worldToScreen(vec2 worldPos) {
    // Transformar a coordenadas de clip
    vec4 clipPos = uViewProj * vec4(worldPos, 0.0, 1.0);
    
    // Convertir a coordenadas normalizadas [0, 1]
    vec2 ndc = (clipPos.xy / clipPos.w) * 0.5 + 0.5;
    
    // Escalar al tamaño de pantalla real
    return ndc * uScreenSize;
}

void main() {
    float pixelScale = (uScreenSize.x / uBaseCamWidth) * uZoom;
    vec2 uv = gl_FragCoord.xy / uScreenSize;
    vec4 sceneColor = texture(uMainTexture, uv);
    
    // Start with complete darkness
    vec3 finalColor = vec3(0.0);
    float totalLightInfluence = 0.0;
    
    for (int i = 0; i < uNumLights; ++i) {
        Light light = uLights[i];
        vec2 fragPos = gl_FragCoord.xy;
        float attenuation = 0.0;
        
        if (light.type == 0) { // Environmental Light
            attenuation = light.intensity;
            totalLightInfluence = max(totalLightInfluence, attenuation);
            finalColor += sceneColor.rgb * light.color * attenuation;
        }
        else if (light.type == 1) { // PointLight
            vec2 lightScreenPos = worldToScreen(light.position);
            float dist = distance(fragPos, lightScreenPos);
            attenuation = smoothAttenuation(dist, light.radius * pixelScale);
            
            if (attenuation > 0.0) {
                totalLightInfluence = max(totalLightInfluence, attenuation);
                finalColor += sceneColor.rgb * light.color * light.intensity * attenuation;
            }
        }
        else if (light.type == 2) { // RayLight
            vec2 start = worldToScreen(light.position);
            vec2 end = worldToScreen(light.endPos);
            float dist = distanceToSegment(fragPos, start, end);
            
            float t = clamp(dot(fragPos - start, end - start) / dot(end - start, end - start), 0.0, 1.0);
            float currentRadius = mix(light.startRadius, light.endRadius, t) * pixelScale;
            
            attenuation = smoothAttenuation(dist, currentRadius);
            
            if (attenuation > 0.0) {
                totalLightInfluence = max(totalLightInfluence, attenuation);
                finalColor += sceneColor.rgb * light.color * light.intensity * attenuation;
            }
        }
    }
    if (totalLightInfluence > 0.0) {
        FragColor = vec4(finalColor, sceneColor.a);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, sceneColor.a);
	}
}
)glsl";
    lightShaderProgram = CreateShaderProgram(lightVS, lightFS);
    
    float quad[] = { -1, -1, 1, -1, -1, 1, 1, 1 };
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);



	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	return ret;
}

bool RendererEM::PreUpdate()
{

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	unsigned int w, h;
	engine->window_em->GetWindowSize(w, h);
	if (w != fbSize.x || h != fbSize.y) {
		ResizeFBO(w, h);
	}
	glViewport(0, 0, w, h);

	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return true;
}

bool RendererEM::Update(double dt)
{
	bool ret = true;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	RenderBatch();
	RenderDebugColliders();
	RenderLights();

	glDisable(GL_BLEND);

	return ret;
}

bool RendererEM::PostUpdate()
{
	if (engine->GetEditorOrBuild())
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		int w, h;
		SDL_GetWindowSize(engine->window_em->GetSDLWindow(), &w, &h);
		glViewport(0, 0, w, h);
	}
	else
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(
			0, 0, fbSize.x, fbSize.y,
			0, 0, fbSize.x, fbSize.y,
			GL_COLOR_BUFFER_BIT, GL_NEAREST
		);
		SDL_GL_SwapWindow(engine->window_em->GetSDLWindow());
	}
	return true;
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
	glDeleteProgram(lightShaderProgram);

	return ret;
}

bool RendererEM::CompileShaders()
{
	bool ret = true;
	const char* vertexShaderSource = R"glsl(
			#version 330 core
			layout (location = 0) in vec2 aPos;
			layout (location = 1) in vec2 aTexCoords;
			
			// Atributos instanciados (1 por instancia)
			layout (location = 2) in mat4 aModel;      // Ocupa locations 2-5
			layout (location = 6) in vec4 aUVRect;     // u1, v1, u2, v2
			layout (location = 7) in vec4 aColor;      // Color de la instancia
			
			uniform mat4 uViewProj;
			
			out vec2 TexCoords;
			out vec4 vColor;
			
			void main() {
			    // Aplica transformaciones
			    mat4 model = aModel;
			    gl_Position = uViewProj * model * vec4(aPos, 0.0, 1.0);
			    
			    // Calcula UVs
			    vec2 uvOffset = aUVRect.xy;
			    vec2 uvScale = aUVRect.zw - aUVRect.xy;
			    TexCoords = uvOffset + aTexCoords * uvScale;
			    
			    vColor = aColor;
			}
        )glsl";

	const char* fragmentShaderSource = R"glsl(
			#version 330 core
			in vec2 TexCoords;
			in vec4 vColor;
			out vec4 FragColor;
			
			uniform sampler2D uTexture;
			
			void main() {
			    vec4 texColor = texture(uTexture, TexCoords);
			    FragColor = texColor * vColor;
			    
			    if (FragColor.a < 0.1)
			        discard;
			}
        )glsl";

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		LOG(LogType::LOG_ERROR, "RendererEM: Vertex shader compilation failed: {}", infoLog);
		return false;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		LOG(LogType::LOG_ERROR, "RendererEM: Fragment shader compilation failed: {}", infoLog);
		return false;
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		LOG(LogType::LOG_ERROR, "RendererEM: Shader program linking failed: {}", infoLog);
		return false;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return ret;
}

void RendererEM::RenderBatch()
{
	glUseProgram(shaderProgram);
	glBindVertexArray(quadVAO);

	glm::mat4 viewProj;

	if (use_scene_cam && engine->GetEditorOrBuild()) {
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

	glUseProgram(shaderProgram);
	GLuint uViewProjLoc = glGetUniformLocation(shaderProgram, "uViewProj");
	GLuint uModelLoc = glGetUniformLocation(shaderProgram, "uModel");
	GLuint uTextureLoc = glGetUniformLocation(shaderProgram, "uTexture");
	GLuint uUVRectLoc = glGetUniformLocation(shaderProgram, "uUVRect");

	glUniformMatrix4fv(uViewProjLoc, 1, GL_FALSE, glm::value_ptr(viewProj));
	glUniform1i(uTextureLoc, 0);

	for (const auto& entry : spritesToRender) {
		const auto& key = entry.first;
		GLuint textureID = key.first;
		bool pixelArt = key.second;
		const auto& sprites = entry.second;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		if (pixelArt) glBindSampler(0, samplerNearest);
		else glBindSampler(0, samplerLinear);

		std::vector<glm::mat4> models;
		std::vector<glm::vec4> uvRects;
		std::vector<glm::vec4> colors;
		for (const auto& sprite : sprites) {
			models.push_back(sprite.modelMatrix);
			uvRects.emplace_back(sprite.u1, sprite.v1, sprite.u2, sprite.v2);
			colors.push_back(sprite.color);
		}

		glBindBuffer(GL_ARRAY_BUFFER, instanceModelVBO);
		glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), models.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, instanceUVRectVBO);
		glBufferData(GL_ARRAY_BUFFER, uvRects.size() * sizeof(glm::vec4), uvRects.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, instanceColorVBO);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_DRAW);

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, sprites.size());
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

	glBindTexture(GL_TEXTURE_2D, lightRenderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glBindRenderbuffer(GL_RENDERBUFFER, rboLightsDepth);
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

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// UVs
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(quadVAO);

	glGenBuffers(1, &instanceModelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceModelVBO);
	for (int i = 0; i < 4; ++i) {
		glEnableVertexAttribArray(2 + i);
		glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
		glVertexAttribDivisor(2 + i, 1);
	}

	glGenBuffers(1, &instanceUVRectVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceUVRectVBO);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
	glVertexAttribDivisor(6, 1);

	glGenBuffers(1, &instanceColorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceColorVBO);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
	glVertexAttribDivisor(7, 1);

	glBindVertexArray(0);
}

GLuint RendererEM::CreateShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource)
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		LOG(LogType::LOG_ERROR, "Vertex shader compilation failed: %s", infoLog);
		return 0;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		LOG(LogType::LOG_ERROR, "Fragment shader compilation failed: %s", infoLog);
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		LOG(LogType::LOG_ERROR, "Shader program linking failed: {}", infoLog);
		return 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

void RendererEM::SetupLightRendering()
{
	float quadVertices[] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f,  1.0f
	};

	// VAO/VBO
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void RendererEM::RenderLights()
{
	if (lightsToRender.empty()) return;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo_lights);
	glViewport(0, 0, fbSize.x, fbSize.y);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glUseProgram(lightShaderProgram);


	glm::mat4 viewProj;
	float zoom = 0;
	if (use_scene_cam && engine->GetEditorOrBuild()) {
		viewProj = scene_camera.GetViewProjMatrix();
		zoom = scene_camera.GetZoom();
	}
	else {
		GameObject* cam_go = &engine->scene_manager_em->GetCurrentScene().GetCurrentCameraGO();
		if (cam_go && cam_go->GetComponent<Camera>())
		{
			auto camera = cam_go->GetComponent<Camera>();

			viewProj = camera->GetProjectionMatrix() * camera->GetViewMatrix();
			zoom = camera->GetZoom();
		}
		else
		{
			viewProj = scene_camera.GetViewProjMatrix();
			zoom = scene_camera.GetZoom();
		}
	}

	float baseCamWidth = fbSize.x;

	GLuint uZoomLoc = glGetUniformLocation(lightShaderProgram, "uZoom");
	GLuint uBaseCamWidthLoc = glGetUniformLocation(lightShaderProgram, "uBaseCamWidth");
	glUniform1f(uZoomLoc, zoom);
	glUniform1f(uBaseCamWidthLoc, baseCamWidth);

	glUniformMatrix4fv(
		glGetUniformLocation(lightShaderProgram, "uViewProj"),
		1,
		GL_FALSE,
		glm::value_ptr(viewProj)
	);

	glm::vec2 screenSize(fbSize.x, fbSize.y);
	glUniform2fv(glGetUniformLocation(lightShaderProgram, "uScreenSize"), 1, &screenSize[0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glUniform1i(glGetUniformLocation(lightShaderProgram, "uMainTexture"), 0);

	glm::vec3 ambientColor(0.0f);
	glUniform3fv(glGetUniformLocation(lightShaderProgram, "uAmbientLight"), 1, &ambientColor[0]);

	GLint maxLights;
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &maxLights);
	int numLights = std::min((int)lightsToRender.size(), 32);

	for (int i = 0; i < numLights; ++i) {
		const LightRenderData& light = lightsToRender[i];
		std::string lightUniform = "uLights[" + std::to_string(i) + "]";

		glUniform2fv(glGetUniformLocation(lightShaderProgram, (lightUniform + ".position").c_str()), 1, &light.position[0]);
		glUniform2fv(glGetUniformLocation(lightShaderProgram, (lightUniform + ".endPos").c_str()), 1, &light.endPosition[0]);
		glUniform3fv(glGetUniformLocation(lightShaderProgram, (lightUniform + ".color").c_str()), 1, &light.color[0]);
		glUniform1f(glGetUniformLocation(lightShaderProgram, (lightUniform + ".intensity").c_str()), light.intensity);
		glUniform1f(glGetUniformLocation(lightShaderProgram, (lightUniform + ".radius").c_str()), light.radius);
		glUniform1f(glGetUniformLocation(lightShaderProgram, (lightUniform + ".startRadius").c_str()), light.startRadius);
		glUniform1f(glGetUniformLocation(lightShaderProgram, (lightUniform + ".endRadius").c_str()), light.endRadius);
		glUniform1i(glGetUniformLocation(lightShaderProgram, (lightUniform + ".type").c_str()), light.type);
	}
	glUniform1i(glGetUniformLocation(lightShaderProgram, "uNumLights"), numLights);

	glViewport(0, 0, fbSize.x, fbSize.y);

	glBindVertexArray(lightVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);

	lightsToRender.clear();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_lights);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glBlitFramebuffer(
		0, 0, fbSize.x, fbSize.y,
		0, 0, fbSize.x, fbSize.y,
		GL_COLOR_BUFFER_BIT, GL_NEAREST
	);
}

void RendererEM::SubmitLight(const LightRenderData& light)
{
	if (light.type == 0)
		for (const auto& item : lightsToRender)
			if (item.type == 0)
				return;

	if (lightsToRender.size() == 32)
		return;

	lightsToRender.push_back(light);
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
	const int FILLED_CIRCLE_SEGMENTS = 32;
	std::vector<float> filledCircleVertices;
	filledCircleVertices.push_back(0.0f);
	filledCircleVertices.push_back(0.0f);
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

	float filledQuadVertices[] = {
		-0.5f,  0.5f, // V0
		 0.5f,  0.5f, // V1
		-0.5f, -0.5f, // V2

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

	float outlineQuadVertices[] = {
		-0.5f,  0.5f, // V0
		 0.5f,  0.5f, // V1
		 0.5f, -0.5f, // V2
		-0.5f, -0.5f, // V3
		-0.5f,  0.5f  // V0
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

	filledCircleVertexCount = filledCircleVertices.size() / 2;
	outlineCircleVertexCount = outlineCircleVertices.size() / 2;
	filledQuadVertexCount = 6;
	outlineQuadVertexCount = 5;
}

void RendererEM::SetupInstancedAttributes(GLuint VAO)
{
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, modelMatricesBuffer);
	for (int i = 0; i < 4; ++i) {
		glEnableVertexAttribArray(1 + i);
		glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
		glVertexAttribDivisor(1 + i, 1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
	glVertexAttribDivisor(5, 1);
}

void RendererEM::SubmitSprite(GLuint textureID, const mat3f& modelMatrix, float u1, float v1, float u2, float v2, bool pixel_art, int order_in_layer, int order_in_component)
{
	auto key = std::make_pair(textureID, pixel_art);

	int layer_index = order_in_layer / 10000;
	int game_object_index = order_in_layer % 10000;

	float pos_z = 10.0f +
		(float)layer_index * 1.0f +
		(float)game_object_index * 0.001f +
		(float)(1000 - order_in_component) * 0.000001f;

	spritesToRender[key].push_back({
		textureID,
		ConvertMat3fToGlmMat4(modelMatrix, -pos_z),
		u1, v1, u2, v2,
		pixel_art,
		{ 1.0f, 1.0f, 1.0f, 1.0f }
		});
}

void RendererEM::SubmitDebugCollider(const mat3f& modelMatrix, const ML_Color& color, bool isCircle, int order_in_layer, int order_in_component, float radius, bool filled)
{
	int layer_index = order_in_layer / 10000;
	int game_object_index = order_in_layer % 10000;

	float pos_z = 10.0f +
		(float)layer_index * 1.0f +
		(float)game_object_index * 0.001f +
		(float)(1000 - order_in_component) * 0.000001f;

	if (isCircle)
	{
		RenderShapeInfo i;
		i.mat = mat3f::CreateTransformMatrix(modelMatrix.GetTranslation(), modelMatrix.GetRotation(), { radius, radius });
		i.color = color;
		i.layer_order = -pos_z;
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
		i.layer_order = -pos_z;
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

void RendererEM::SubmitText(std::string text, FontData* font, const mat3f& modelMatrix, const ML_Color& color, TextAlignment ta, int order_in_layer, int order_in_component)
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

			auto key = std::make_pair(ch->textureID, true);

			int layer_index = order_in_layer / 10000;
			int game_object_index = order_in_layer % 10000;

			float pos_z = 10.0f +
				(float)layer_index * 1.0f +
				(float)game_object_index * 0.001f +
				(float)(1000 - order_in_component) * 0.000001f;

			spritesToRender[key].push_back({
				ch->textureID,
				ConvertMat3fToGlmMat4(charModel, -pos_z),
				0.0f, 1.0f,
				1.0f, 0.0f,
				true,
				{
					color.r / 255.0f,
					color.g / 255.0f,
					color.b / 255.0f,
					color.a / 255.0f
				}
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