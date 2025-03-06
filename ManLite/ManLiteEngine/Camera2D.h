#pragma once
#include <glm/glm.hpp>

class Camera2D {
public:
    Camera2D(int displayWidth, int displayHeight);

    void SetPosition(const glm::vec2& newPosition);
    void SetZoom(float newZoom);

    void Move(const glm::vec2& movement);
    void Zoom(float factor);

    void Resize(int width, int height);

    const glm::mat4& GetViewProjMatrix() const { return viewProjMatrix; }
    const glm::vec2& GetPosition() const { return position; }
    const float GetZoom() const { return zoom; }
    const int GetWidth() const { return width; }
    const int GetHeight() const { return height; }
    glm::vec2 GetVisibleRange() const;

private:
    void UpdateMatrix();

    glm::vec2 position = glm::vec2(0.0f);
    float zoom = 1.0f;
    int width, height;
    glm::mat4 viewProjMatrix;
};