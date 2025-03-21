#ifndef __CAMERA_H__
#define __CAMERA_H__
#pragma once

#include "Component.h"
#include "Transform.h"

class Camera : public Component {
public:
    Camera(std::weak_ptr<GameObject> container_go, std::string name = "Camera", bool enable = true);
    Camera(const Camera& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Camera();

private:

};

#endif // !__CAMERA_H__