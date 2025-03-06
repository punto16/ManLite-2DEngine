#ifndef __RENDERER_EM_H__
#define __RENDERER_EM_H__
#pragma once

#include "EngineModule.h"

#include "Camera2D.h"

#include <GL/glew.h>
#include "SDL2/SDL.h"

#include "Defs.h"

class Grid
{
public:
	Grid(float size, int divisions);
	void Draw(const glm::mat4& viewProjMatrix);

private:
	GLuint vao, vbo;
	GLuint shaderProgram;
	float gridSize;
	int gridDivisions;

	const char* gridVertexShader = R"glsl(
    #version 330 core
	layout (location = 0) in vec2 aPos;
	uniform mat4 uViewProj;
	
	out vec2 vWorldPos;
	
	void main() {
	    gl_Position = uViewProj * vec4(aPos, 0.0, 1.0);
	    vWorldPos = aPos;  // Pasamos las coordenadas del mundo al fragment shader
	}
)glsl";

	const char* gridFragmentShader = R"glsl(
	#version 330 core
	in vec2 vWorldPos; // Coordenadas del mundo SIN la cámara
	out vec4 FragColor;
	
	uniform vec3 uGridColor = vec3(0.5);
	uniform vec3 uAxisColor = vec3(1.0, 0.0, 0.0);
	uniform float uZoom;
	uniform vec2 uCameraPos; 
	uniform vec2 uVisibleRange;
	
	const float AXIS_THICKNESS = 2.0; 
	const float DOT_RADIUS = 3.0;
	
	void main() {
	    // 1. Posición real en el mundo (sin ajustar por la cámara)
	    vec2 worldPos = vWorldPos;
	    
	    // 2. Distancia desde la posición de la cámara
	    vec2 delta = worldPos - uCameraPos;
	    
	    // 3. Early exit (usando la posición real del mundo)
	    if (abs(delta.x) > uVisibleRange.x || abs(delta.y) > uVisibleRange.y) discard;
	    
	    // 4. Ejes centrales (en el origen del mundo, no de la cámara)
	    float axisX = smoothstep(AXIS_THICKNESS / uZoom, 0.0, abs(worldPos.y));
	    float axisY = smoothstep(AXIS_THICKNESS / uZoom, 0.0, abs(worldPos.x));
	    
	    // 5. Punto central (en el origen del mundo)
	    float dot = 1.0 - smoothstep(DOT_RADIUS - 0.5, DOT_RADIUS + 0.5, length(worldPos * uZoom));
	    
	    // 6. Grid (coherente con la cámara)
	    vec2 gridPos = worldPos / pow(2.0, floor(log2(uZoom)));
	    vec2 grid = abs(fract(gridPos - 0.5) - 0.5) / fwidth(gridPos);
	    float line = min(grid.x, grid.y);
	    
	    // 7. Mezclar colores
	    vec3 color = mix(uGridColor, uAxisColor, max(axisX, axisY));
	    color = mix(color, vec3(0.0), dot);
	    color = mix(color, uGridColor * 0.7, 1.0 - smoothstep(0.0, 1.0, line));
	    
	    FragColor = vec4(color, 1.0);
	}
)glsl";
};

struct Vertex
{
	vec3f position;
	vec4f color;
	vec2f texCoords;
};

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

	bool CompileShaders();

	void RenderBatch();
	void ResizeFBO(int width, int height);

	mat4f GetProjection() const { return this->projection; }
	Camera2D& GetSceneCamera() { return scene_camera; }

private:

	bool vsync;

	SDL_GLContext glContext;

	GLuint VAO, VBO, EBO;
	GLuint shaderProgram;
	mat4f projection;
	Camera2D scene_camera;

public:
	GLuint fbo;
	GLuint renderTexture;
	GLuint rbo;
	glm::ivec2 fbSize = { DEFAULT_CAM_WIDTH, DEFAULT_CAM_HEIGHT };
};

#endif // !__EWINDOW_EM_H__