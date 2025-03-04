#include "SceneManagerEM.h"

//SCENE MANAGER
SceneManagerEM::SceneManagerEM(EngineCore* parent) : EngineModule(parent)
{
}

SceneManagerEM::~SceneManagerEM()
{
}

bool SceneManagerEM::Awake()
{
	bool ret = true;

	return ret;
}

bool SceneManagerEM::Start()
{
	bool ret = true;

	return ret;
}

bool SceneManagerEM::PreUpdate()
{
	bool ret = true;

	return ret;
}

bool SceneManagerEM::Update(double dt)
{
	bool ret = true;

	return ret;
}

bool SceneManagerEM::PostUpdate()
{
	bool ret = true;

	return ret;
}

bool SceneManagerEM::CleanUp()
{
	bool ret = true;

	return ret;
}

//SCENE
Scene::Scene(std::string scene_name) : scene_name(scene_name)
{
}

Scene::~Scene()
{
}

bool Scene::Update(double dt)
{
	bool ret = true;

	return ret;
}
