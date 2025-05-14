#include "Camera.h"
#include "EngineCore.h"
#include "SceneManagerEM.h"
#include "Transform.h"
#include "GameObject.h"
#include "RendererEM.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp> 

Camera::Camera(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
    Component(container_go, ComponentType::Camera, name, enable)
{
    SetViewportSize(DEFAULT_CAM_WIDTH, DEFAULT_CAM_HEIGHT);
}

Camera::Camera(const Camera& component_to_copy, std::shared_ptr<GameObject> container_go) :
    Component(component_to_copy, container_go),
    viewport_width(component_to_copy.viewport_width),
    viewport_height(component_to_copy.viewport_height),
    zoom(component_to_copy.zoom)
{
    UpdateMatrices();
}

Camera::~Camera()
{
}

void Camera::Draw()
{
    //if (!engine->GetEditorOrBuild()) return;
    if (auto transform = container_go.lock()->GetComponent<Transform>())
    {
        vec2f o_scale = transform->GetWorldScale();
        bool a_lock = transform->IsAspectRatioLocked();
        transform->SetAspectRatioLock(false);
        transform->SetWorldScale({ 1.0f, 1.0f });

        mat3f worldMat = transform->GetWorldMatrix();

        transform->SetAspectRatioLock(a_lock);
        transform->SetWorldScale(o_scale);

        ML_Color color = { 255, 255, 255, 255 };

        float half_width = (viewport_width) / zoom;
        float half_height = (viewport_height) / zoom;

        mat3f colliderMat = mat3f::CreateTransformMatrix(
            vec2f(0, 0),
            0.0f,
            { half_width, half_height }
        );

        mat3f finalMat = worldMat * colliderMat;
        engine->renderer_em->SubmitDebugCollider(finalMat, color, false, engine->scene_manager_em->GetCurrentScene().GetGOOrderInLayer(container_go.lock()));
    }
}

nlohmann::json Camera::SaveComponent()
{
    nlohmann::json componentJSON;
    //component generic
    componentJSON["ContainerGOID"] = this->container_go.lock()->GetID();
    componentJSON["ComponentID"] = component_id;
    componentJSON["ComponentName"] = name;
    componentJSON["ComponentType"] = (int)type;
    componentJSON["Enabled"] = enabled;

    //component spcecific
    componentJSON["ViewportSize"] = { viewport_width, viewport_height };
    componentJSON["CameraZoom"] = zoom;

    return componentJSON;
}

void Camera::LoadComponent(const nlohmann::json& componentJSON)
{
    if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
    if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
    if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
    if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];

    if (componentJSON.contains("ViewportSize")) SetViewportSize(componentJSON["ViewportSize"][0], componentJSON["ViewportSize"][1]);
    if (componentJSON.contains("CameraZoom")) SetZoom(componentJSON["CameraZoom"]);
    UpdateMatrices();
}

void Camera::SetViewportSize(int width, int height)
{
    viewport_width = width;
    viewport_height = height;
    UpdateMatrices();
}

void Camera::GetViewportSize(int& width, int& height)
{
    width = this->viewport_width;
    height = this->viewport_height;
}

void Camera::SetZoom(float new_zoom)
{
    zoom = glm::clamp(new_zoom, 1.0f, 1000.0f);
    UpdateMatrices();
}

void Camera::Resize(unsigned int width, unsigned int height)
{
    this->viewport_width = width;
    this->viewport_height = height;
    UpdateMatrices();
}

glm::mat4 Camera::GetViewMatrix() const
{
    if (auto transform = container_go.lock()->GetComponent<Transform>())
    {
        vec2f o_scale = transform->GetWorldScale();
        bool a_lock = transform->IsAspectRatioLocked();
        transform->SetAspectRatioLock(false);
        transform->SetWorldScale({ 1.0f, 1.0f });

        mat3f worldMat = transform->GetWorldMatrix();

        transform->SetAspectRatioLock(a_lock);
        transform->SetWorldScale(o_scale);
        glm::mat4 viewMat = engine->renderer_em->ConvertMat3fToGlmMat4(worldMat.Inverted());
        return viewMat;
    }
    return glm::mat4(1.0f);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return projection_matrix;
}

glm::mat4 Camera::GetViewProjMatrix() const
{
    return projection_matrix * GetViewMatrix();
}

void Camera::UpdateMatrices()
{
    float half_width = (viewport_width * 0.5f) / zoom;
    float half_height = (viewport_height * 0.5f) / zoom;

    projection_matrix = glm::ortho(
        -half_width, half_width,
        -half_height, half_height,
        -1000.0f, 1000.0f
    );
}