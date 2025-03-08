#include "Camera2D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera2D::Camera2D(int displayWidth, int displayHeight)
    : width(displayWidth), height(displayHeight) {
    UpdateMatrix();
}

void Camera2D::SetPosition(const glm::vec2& newPosition) {
    position = newPosition;
    UpdateMatrix();
}

void Camera2D::SetZoom(float newZoom) {
    zoom = glm::clamp(newZoom, 10.0f, 200.0f);
    UpdateMatrix();
}

void Camera2D::Move(const glm::vec2& movement)
{
    position += movement;
    UpdateMatrix();
}

void Camera2D::Zoom(float factor) {
    zoom = glm::clamp(zoom * factor, 10.0f, 200.0f);
    UpdateMatrix();
}

void Camera2D::Resize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    UpdateMatrix();
}

glm::vec2 Camera2D::GetVisibleRange() const
{
    return glm::vec2(
        (width * 0.5f) / zoom,
        (height * 0.5f) / zoom
    );
}

void Camera2D::UpdateMatrix() {
    glm::mat4 projection = glm::ortho(
        -width * 0.5f / zoom,
        width * 0.5f / zoom,
        -height * 0.5f / zoom,
        height * 0.5f / zoom,
        -1.0f, 1.0f
    );

    glm::mat4 view = glm::translate(glm::mat4(1.0f),
        glm::vec3(-position, 0.0f));

    //LOG(LogType::LOG_INFO, "Camera Updated - Zoom: %.2f, Pos: (%.1f, %.1f)", zoom, position.x, position.y);

    viewProjMatrix = projection * view;
}