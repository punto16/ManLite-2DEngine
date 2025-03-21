#include "Camera.h"
#include "EngineCore.h"
#include "SceneManagerEM.h"

Camera::Camera(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
	Component(container_go, ComponentType::Camera, name, enable)
{
	engine->scene_manager_em->GetCurrentScene().SetCamerasInSceneAmount(engine->scene_manager_em->GetCurrentScene().GetCamerasInSceneAmount() + 1);
}

Camera::Camera(const Camera& component_to_copy, std::shared_ptr<GameObject> container_go) :
	Component(component_to_copy, container_go)
{
	engine->scene_manager_em->GetCurrentScene().SetCamerasInSceneAmount(engine->scene_manager_em->GetCurrentScene().GetCamerasInSceneAmount() + 1);
}

Camera::~Camera()
{
	engine->scene_manager_em->GetCurrentScene().SetCamerasInSceneAmount(engine->scene_manager_em->GetCurrentScene().GetCamerasInSceneAmount() - 1);
}
