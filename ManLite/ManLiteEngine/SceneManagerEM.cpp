#include "SceneManagerEM.h"

#include "GameObject.h"
#include "Layer.h"

#include "Defs.h"

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

	if (!current_scene->CleanUp()) return false;

	return ret;
}

//SCENE
Scene::Scene(std::string scene_name) : scene_name(scene_name)
{
	this->scene_root = std::make_shared<GameObject>(scene_root, scene_name, true);
	this->scene_root->Awake();
	this->scene_layers.push_back(std::make_shared<Layer>(scene_layers.size(), "DefaultLayer_" + std::to_string(scene_layers.size()), true));
	LOG(LogType::LOG_INFO, "Scene <%s> created", scene_name.c_str());
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

bool Scene::CleanUp()
{
	bool ret = true;

	objects_to_add.clear();
	layers_to_add.clear();
	objects_to_delete.clear();
	layers_to_delete.clear();
	scene_layers.clear();

	scene_root->Delete();

	return ret;
}

std::shared_ptr<GameObject> Scene::CreateEmptyGO(GameObject& parent)
{
	std::shared_ptr<GameObject> empty_go = std::make_shared<GameObject>(parent.shared_from_this(), "EmptyGameObject", true);
	empty_go->Awake();
	parent.AddChild(empty_go);
	if (scene_layers.size() > 0) ReparentToLayer(empty_go, scene_layers[0]);
	return empty_go;
}

std::shared_ptr<GameObject> Scene::DuplicateGO(GameObject& go_to_copy)
{
	std::shared_ptr<GameObject> copy = std::make_shared<GameObject>(go_to_copy.GetSharedPtr());
	copy->CloneComponents(go_to_copy.GetSharedPtr());
	go_to_copy.GetParentGO().lock()->AddChild(copy);
	if (go_to_copy.GetParentLayer().lock() != nullptr) ReparentToLayer(copy, go_to_copy.GetParentLayer().lock());

	if (!go_to_copy.GetChildren().empty());
	for (const auto& child_to_copy : go_to_copy.GetChildren())
		DuplicateGO(*child_to_copy)->Reparent(copy);

	return copy;
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
			break;
		}
	}
	if (target_layer)
	{
		game_object->SetParentLayer(target_layer);
		target_layer->GetChildren().push_back(game_object);
	}
}

bool Scene::ContainsLayer(const std::shared_ptr<Layer>& layer) const {
	return std::find(scene_layers.begin(), scene_layers.end(), layer) != scene_layers.end();
}

void Scene::SelectGameObject(std::shared_ptr<GameObject> go, bool additive, bool unselect_all)
{
	if (unselect_all)
	{
		selected_gos.clear();
		last_selected = nullptr;
		return;
	}
	if (!additive) selected_gos.clear();

	auto it = std::find_if(selected_gos.begin(), selected_gos.end(),
		[&](const auto& weak_go) { return weak_go.lock() == go; });

	if (it == selected_gos.end())
	{
		selected_gos.push_back(go);
		last_selected = go;
	}
}

void Scene::SelectRange(std::shared_ptr<GameObject> endGO)
{
	if (!last_selected)
	{
		SelectGameObject(endGO);
		return;
	}

	std::vector<std::shared_ptr<GameObject>> flatHierarchy;
	TraverseHierarchy([&](auto go) { flatHierarchy.push_back(go); });

	auto startIt = std::find(flatHierarchy.begin(), flatHierarchy.end(), last_selected);
	auto endIt = std::find(flatHierarchy.begin(), flatHierarchy.end(), endGO);

	if (startIt == flatHierarchy.end() || endIt == flatHierarchy.end()) return;

	if (startIt > endIt) std::swap(startIt, endIt);

	selected_gos.clear();
	for (auto it = startIt; it <= endIt; ++it)
		selected_gos.push_back(*it);

	last_selected = endGO;
}

void Scene::TraverseRecursive(std::shared_ptr<GameObject> go, const std::function<void(std::shared_ptr<GameObject>)>& func)
{
	if (!go) return;

	func(go);

	const auto& children = go->GetChildren();
	for (auto it = children.rbegin(); it != children.rend(); ++it)
		TraverseRecursive(*it, func);
}

void Scene::SetSceneName(std::string name)
{
	this->scene_name = name;
	scene_root->SetName(name);
}
