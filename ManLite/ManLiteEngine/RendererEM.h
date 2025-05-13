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
#include "string"

#include "mat3f.h"

class FontData;

struct SpriteRenderData {
    GLuint textureID;
    glm::mat4 modelMatrix;
    float u1, v1, u2, v2;
    bool pixel_art;
    glm::vec4 color;
    bool text;
};

enum TextAlignment
{
    TEXT_ALIGN_LEFT             = 0,
    TEXT_ALIGN_CENTER           = 1,
    TEXT_ALIGN_RIGHT            = 2
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
    
    const float AXIS_THICKNESS = 2.0;
    const float DOT_RADIUS = 3.0;
    const float BASE_GRID_THICKNESS = 1.0;
    const float MAX_GRID_THICKNESS = 0.6;
    
    void main() {
        float axisWidth = (AXIS_THICKNESS / uZoom) * 0.5;
        float axisX = step(abs(vWorldPos.y), axisWidth);
        float axisY = step(abs(vWorldPos.x), axisWidth);
        
        float dotRadius = DOT_RADIUS / uZoom;
        float distToCenter = length(vWorldPos);
        float dot = step(distToCenter, dotRadius);
        
        float baseThickness = BASE_GRID_THICKNESS / uZoom;
        float lineThickness = min(baseThickness, MAX_GRID_THICKNESS);
        
        vec2 gridPos = vWorldPos / uStepSize;
        vec2 gridFract = fract(gridPos);
        vec2 distToLine = min(gridFract, 1.0 - gridFract) * uStepSize;
        
        float gridLineX = step(distToLine.x, lineThickness);
        float gridLineY = step(distToLine.y, lineThickness);
        float gridLine = max(gridLineX, gridLineY);
        
        vec3 color = uGridColor * gridLine;
        color = mix(color, uAxisColor, max(axisX, axisY));
        color = mix(color, vec3(0.0), dot);
        
        float alpha = max(max(axisX, axisY), max(dot, gridLine));
        FragColor = vec4(color, alpha * 0.8); // 80% de opacidad
    }
    )glsl";
};

struct RenderCircleInfo
{
    mat3f mat;
    ML_Color color;
    float radius;
};

struct RenderRectInfo
{
    mat3f mat;
    ML_Color color;
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

    void UseSceneViewCam();
    void UseGameViewCam();

    void SetupDebugShapes();
    void SetupInstancedAttributes(GLuint VAO);
    void SubmitSprite(GLuint textureID, const mat3f& modelMatrix, float u1, float v1, float u2, float v2, bool pixel_art);
    void SubmitDebugCollider(const mat3f& modelMatrix, const ML_Color& color, bool isCircle, float radius = 0.0f, bool filled = false);
    void RenderDebugColliders();
    template<typename T>
    void RenderBatchShapes(const std::vector<T>& instances, GLuint VAO, GLenum mode, int vertexCount);
    void SubmitText(std::string text, FontData* font, const mat3f& modelMatrix, const ML_Color& color, TextAlignment ta = TextAlignment::TEXT_ALIGN_LEFT);

    static glm::mat4 ConvertMat3fToGlmMat4(const mat3f& mat);
private:

	bool vsync;

	SDL_GLContext glContext;

    GLuint samplerLinear;
    GLuint samplerNearest;

	GLuint VAO, VBO, EBO;
	GLuint shaderProgram;
	mat4f projection;
	Camera2D scene_camera;

    bool use_scene_cam = true;

    std::vector<SpriteRenderData> spritesToRender;
    GLuint quadVAO, quadVBO, quadEBO;

    void SetupQuad();
public:
	GLuint fbo;
	GLuint renderTexture;
	GLuint rbo;
	glm::ivec2 fbSize = { DEFAULT_CAM_WIDTH, DEFAULT_CAM_HEIGHT };

    //collider stuff
    GLuint debugShaderProgram;
    GLuint lineVAO, lineVBO;
    std::vector<RenderCircleInfo> debugCollidersCircleFilled;
    std::vector<RenderCircleInfo> debugCollidersCircleContorn;
    std::vector<RenderRectInfo> debugCollidersRectFilled;
    std::vector<RenderRectInfo> debugCollidersRectContorn;
    int filledCircleVertexCount;
    int outlineCircleVertexCount;
    int filledQuadVertexCount;
    int outlineQuadVertexCount;
    GLuint modelMatricesBuffer;
    GLuint colorsBuffer;
    GLuint filledCircleVAO, filledCircleVBO;
    GLuint outlineCircleVAO, outlineCircleVBO;
    GLuint filledQuadVAO, filledQuadVBO;
    GLuint outlineQuadVAO, outlineQuadVBO;

    //text
    GLuint textShaderProgram;

    //shaders
    const char* debugVertexShader = R"glsl(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in mat4 modelMatrix; // 4 atributos (location 1-4)
layout(location = 5) in vec4 color;

uniform mat4 uViewProj;

out vec4 vColor;

void main() {
    gl_Position = uViewProj * modelMatrix * vec4(aPos, 0.0, 1.0);
    vColor = color;
}
)glsl";
    
    const char* debugFragmentShader = R"glsl(
#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main() {
    FragColor = vColor;
}
)glsl";


    const char* textVertexShader = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoords;
    
    uniform mat4 uModel;
    uniform mat4 uViewProj;
    uniform vec4 uUVRect;
    
    out vec2 TexCoords;
    
    void main() {
        gl_Position = uViewProj * uModel * vec4(aPos, 0.0, 1.0);
        vec2 uvOffset = vec2(uUVRect.x, uUVRect.y);
        vec2 uvScale = vec2(uUVRect.z - uUVRect.x, uUVRect.w - uUVRect.y);
        TexCoords = uvOffset + aTexCoords * uvScale;
    }
    )glsl";

    const char* textFragmentShader = R"glsl(
    #version 330 core
    in vec2 TexCoords;
    out vec4 FragColor;
    
    uniform sampler2D uTexture;
    uniform vec4 uTextColor;
    
    void main() {
        vec4 texColor = texture(uTexture, TexCoords);
        FragColor = vec4(uTextColor.rgb, uTextColor.a * texColor.a);
        
        if (FragColor.a < 0.1)
            discard;
    }
    )glsl";
};

#endif // !__RENDERER_EM_H__

template<typename T>
inline void RendererEM::RenderBatchShapes(const std::vector<T>& instances, GLuint VAO, GLenum mode, int vertexCount)
{
    // Convertir datos a formatos de GPU
    std::vector<glm::mat4> modelMatrices;
    std::vector<glm::vec4> colors;
    for (const auto& inst : instances) {
        modelMatrices.push_back(ConvertMat3fToGlmMat4(inst.mat));
        colors.emplace_back(
            inst.color.r / 255.0f,
            inst.color.g / 255.0f,
            inst.color.b / 255.0f,
            inst.color.a / 255.0f
        );
    }

    // Actualizar buffers de instancias
    glBindBuffer(GL_ARRAY_BUFFER, modelMatricesBuffer);
    glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), modelMatrices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_DRAW);

    // Dibujar
    glBindVertexArray(VAO);
    glDrawArraysInstanced(mode, 0, vertexCount, instances.size());
}
