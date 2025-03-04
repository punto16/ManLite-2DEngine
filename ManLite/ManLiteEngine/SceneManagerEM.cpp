#include "SceneManagerEM.h"

#include "GameObject.h"
#include "Layer.h"

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

	this->current_scene = std::make_unique<Scene>();

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

	if (!current_scene.get()->Update(dt)) return false;

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
	this->scene_root = std::make_shared<GameObject>(nullptr, scene_name, true);
	this->scene_layers.push_back(std::make_shared<Layer>(scene_layers.size(), "Default_Layer_id:" + scene_layers.size(), true));
}

Scene::~Scene()
{
}

bool Scene::Update(double dt)
{
	bool ret = true;

	for (const auto& item : scene_root.get()->GetChildren()) 
		if (!item->Update(dt)) 
			return false;

	for (const auto& item : scene_layers) 
		if (!item->Update(dt)) 
			return false;

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

void Scene::ReparentToLayer(std::shared_ptr<GameObject> game_object, std::shared_ptr<Layer> target_layer)
{
	if (!ContainsLayer(target_layer)) { return; }
	bool was_in_any_layer = false;
	for (auto& layer : scene_layers)
	{
		auto& objects = layer->GetChildren();
		const size_t initial_size = objects.size();

		objects.erase(
			std::remove(objects.begin(), objects.end(), game_object),
			objects.end()
		);

		if (objects.size() != initial_size)
		{
			was_in_any_layer = true;
			break;
		}
	}
	if (was_in_any_layer) {	target_layer->GetChildren().push_back(game_object);	}
}

bool Scene::ContainsLayer(const std::shared_ptr<Layer>& layer) const {
	return std::find(scene_layers.begin(), scene_layers.end(), layer) != scene_layers.end();
}
