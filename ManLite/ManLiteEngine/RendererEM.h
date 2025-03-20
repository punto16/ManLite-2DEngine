#ifndef __RENDERER_EM_H__
#define __RENDERER_EM_H__
#pragma once

#include "EngineModule.h"

#include "Camera2D.h"

#include <GL/glew.h>
#include "SDL2/SDL.h"
#include "glm/glm.hpp"

#include "Defs.h"
#include "vector"

class mat3f;

struct SpriteRenderData {
    GLuint textureID;
    glm::mat4 modelMatrix;
};

class Grid {
public:
    Grid(float size, int divisions);
    void Draw(const glm::mat4& viewProjMatrix);

private:
    GLuint vao, vbo;
    GLuint shaderProgram;
    float stepSize;

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
    
    const float AXIS_THICKNESS = 2.0;   // Grosor ejes en p�xeles
    const float DOT_RADIUS = 3.0;       // Radio del punto en p�xeles
    const float BASE_GRID_THICKNESS = 1.0; // Grosor base en p�xeles
    const float MAX_GRID_THICKNESS = 0.6; // Grosor m�ximo en unidades del mundo
    
    void main() {
        //-------------------------------------------
        // 1. Calcular ejes centrales (l�neas s�lidas)
        //-------------------------------------------
        float axisWidth = (AXIS_THICKNESS / uZoom) * 0.5;
        float axisX = step(abs(vWorldPos.y), axisWidth);
        float axisY = step(abs(vWorldPos.x), axisWidth);
        
        //-------------------------------------------
        // 2. Calcular punto central (c�rculo s�lido)
        //-------------------------------------------
        float dotRadius = DOT_RADIUS / uZoom;
        float distToCenter = length(vWorldPos);
        float dot = step(distToCenter, dotRadius);
        
        //-------------------------------------------
        // 3. Grosor din�mico del grid seg�n el zoom
        //-------------------------------------------
        // Grosor base en unidades del mundo (ajustado por zoom)
        float baseThickness = BASE_GRID_THICKNESS / uZoom;
        // Limitar grosor m�ximo para evitar "bloques" al hacer zoom out
        float lineThickness = min(baseThickness, MAX_GRID_THICKNESS);
        
        //-------------------------------------------
        // 4. Calcular grid (alineado a 0,0)
        //-------------------------------------------
        vec2 gridPos = vWorldPos / uStepSize;
        vec2 gridFract = fract(gridPos);
        vec2 distToLine = min(gridFract, 1.0 - gridFract) * uStepSize;
        
        float gridLineX = step(distToLine.x, lineThickness);
        float gridLineY = step(distToLine.y, lineThickness);
        float gridLine = max(gridLineX, gridLineY);
        
        //-------------------------------------------
        // 5. Mezclar colores y aplicar alpha 80%
        //-------------------------------------------
        vec3 color = uGridColor * gridLine;
        color = mix(color, uAxisColor, max(axisX, axisY));
        color = mix(color, vec3(0.0), dot);
        
        float alpha = max(max(axisX, axisY), max(dot, gridLine));
        FragColor = vec4(color, alpha * 0.8); // 80% de opacidad
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

    void SubmitSprite(GLuint textureID, const mat3f& modelMatrix);

private:

	bool vsync;

	SDL_GLContext glContext;

	GLuint VAO, VBO, EBO;
	GLuint shaderProgram;
	mat4f projection;
	Camera2D scene_camera;


    std::vector<SpriteRenderData> spritesToRender;
    GLuint quadVAO, quadVBO, quadEBO;

    void SetupQuad();
    glm::mat4 ConvertMat3fToGlmMat4(const mat3f& mat);
public:
	GLuint fbo;
	GLuint renderTexture;
	GLuint rbo;
	glm::ivec2 fbSize = { DEFAULT_CAM_WIDTH, DEFAULT_CAM_HEIGHT };
};

#endif // !__RENDERER_EM_H__