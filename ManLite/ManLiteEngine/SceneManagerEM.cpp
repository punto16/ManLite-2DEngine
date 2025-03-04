#include "SceneManagerEM.h"

#include "GameObject.h"

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

	//add pending gos
	current_scene.get()->AddPengindGOs();
	current_scene.get()->AddPengindLayers();

	ret = current_scene.get()->Update(dt);

	//delete pending gos
	current_scene.get()->DeletePengindGOs();
	current_scene.get()->DeletePengindLayers();


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
	//never init smart pointers in constructor
}

Scene::~Scene()
{
}

bool Scene::Update(double dt)
{
	bool ret = true;

	return ret;
}

void Scene::SafeAddGO(std::shared_ptr<GameObject> object_to_add)
{
	objects_to_add.push_back(object_to_add);
}

void Scene::SafeDeleteGO(std::shared_ptr<GameObject> object_to_delete)
{
	objects_to_delete.push_back(object_to_delete);
}

void Scene::SafeAddLayer(std::shared_ptr<Layer> layer_to_add)
{
	layers_to_add.push_back(layer_to_add);
}

void Scene::SafeDeleteLayer(std::shared_ptr<Layer> layer_to_delete)
{
	layers_to_delete.push_back(layer_to_delete);
}

void Scene::AddPengindGOs()
{
	for (const auto& item : objects_to_add) scene_root.get()->GetChildren().push_back(item);
	objects_to_add.clear();
}

void Scene::DeletePengindGOs()
{
	for (const auto& item : objects_to_delete) item.get()->Delete();
	objects_to_delete.clear();
}

void Scene::AddPengindLayers()
{
}

void Scene::DeletePengindLayers()
{
}
