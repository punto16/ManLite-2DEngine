#ifndef __RENDERER_EM_H__
#define __RENDERER_EM_H__
#pragma once

#include "EngineModule.h"

#include "Camera2D.h"

#include <GL/glew.h>
#include "SDL2/SDL.h"

#include "Defs.h"

class Grid {
public:
    Grid(float size, int divisions);
    void Draw(const glm::mat4& viewProjMatrix);

private:
    GLuint vao, vbo;
    GLuint shaderProgram;
    float stepSize;  // Tamaño entre divisiones

    // Shaders
    const char* gridVertexShader = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    uniform mat4 uViewProjInv;
    
    out vec2 vWorldPos;
    
    void main() {
        vec4 worldPos = uViewProjInv * vec4(aPos, 0.0, 1.0);
        vWorldPos = worldPos.xy;
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
    )glsl";

    const char* gridFragmentShader = R"glsl(
    #version 330 core
    in vec2 vWorldPos;
    out vec4 FragColor;
    
    uniform vec3 uGridColor = vec3(0.5);
    uniform vec3 uAxisColor = vec3(1.0, 0.0, 0.0);
    uniform float uZoom;
    uniform float uStepSize;
    
    const float AXIS_THICKNESS = 2.0;
    const float DOT_RADIUS = 3.0;
    
    void main() {
        // Ejes centrales
        float axisX = smoothstep(AXIS_THICKNESS / uZoom, 0.0, abs(vWorldPos.y));
        float axisY = smoothstep(AXIS_THICKNESS / uZoom, 0.0, abs(vWorldPos.x));
        
        // Punto central
        float dot = 1.0 - smoothstep(DOT_RADIUS - 0.5, DOT_RADIUS + 0.5, length(vWorldPos * uZoom));
        
        // Grid basado en stepSize
        vec2 gridPos = vWorldPos / uStepSize;
        vec2 grid = abs(fract(gridPos - 0.5) - 0.5) / fwidth(gridPos);
        float line = min(grid.x, grid.y);
        
        // Mezcla de colores
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