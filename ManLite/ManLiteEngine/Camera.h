#ifndef __CAMERA_H__
#define __CAMERA_H__
#pragma once

#include "Component.h"
#include "Transform.h"
#include "Defs.h"
#include <glm/glm.hpp>

class Camera : public Component {
public:
    Camera(std::weak_ptr<GameObject> container_go, std::string name = "Camera", bool enable = true);
    Camera(const Camera& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Camera();

    //serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    //getters // setters
    void SetViewportSize(int width, int height);
    void GetViewportSize(int &width, int &height);
    void SetZoom(float zoom);
    float GetZoom() const { return zoom; }

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetViewProjMatrix() const;

private:
    void UpdateMatrices();

    int viewport_width = DEFAULT_CAM_WIDTH;
    int viewport_height = DEFAULT_CAM_HEIGHT;
    float zoom = 1.0f;
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
};

#endif // !__CAMERA_H__