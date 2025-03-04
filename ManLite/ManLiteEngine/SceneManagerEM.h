#ifndef __SCENEMANAGER_EM_H__
#define __SCENEMANAGER_EM_H__
#pragma once

#include "EngineModule.h"
#include <string>
#include <memory>

class Scene
{
public:
	Scene(std::string scene_name = "Untitled Scene");
	~Scene();

	bool Update(double dt);

private:
	std::string scene_name;

	//std::shared_ptr<GameObject> scene_root;
	//std::shared_ptr<Layer> layers_root;
};

class SceneManagerEM : public EngineModule
{
public:
	SceneManagerEM(EngineCore* parent);
	virtual ~SceneManagerEM();

	bool Awake();
	bool Start();

	bool PreUpdate();
	bool Update(double dt);
	bool PostUpdate();

	bool CleanUp();

private:
	std::unique_ptr<Scene> current_scene;
};

#endif // !__SCENEMANAGER_EM_H__