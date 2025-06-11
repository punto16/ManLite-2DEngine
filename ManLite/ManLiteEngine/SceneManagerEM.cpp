#include "SceneManagerEM.h"

#include "GameObject.h"
#include "Layer.h"
#include "Sprite2D.h"
#include "EngineCore.h"
#include "PhysicsEM.h"
#include "AudioEM.h"
#include "TileMap.h"
#include "Collider2D.h"
#include "Script.h"
#include "Camera.h"
#include "MonoRegisterer.h"
#include "RendererEM.h"

#include "Defs.h"
#include "Log.h"
#include "algorithm"
#include "filesystem"
#include "fstream"
#include <regex>

namespace fs = std::filesystem;

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

	auto cam_go = current_scene->CreateEmptyGO(current_scene->GetSceneRoot());
	cam_go->SetName("MainCamera");
	cam_go->AddComponent<Camera>();
	current_scene->SetCurrentCameraGO(cam_go);

	ML_Color bg_color = { 102, 102, 102, 255 };
	vec2f world_gravity = { 0.0f, -9.81f };
	current_scene->SetBackGroundColor(bg_color);
	current_scene->SetSceneGravity(world_gravity);

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

	if (!current_scene.get()->Update(dt)) return false;

	if (!scene_to_load.empty())
	{
		bool engine_mode = engine->GetEditorOrBuild();
		engine->SetEditorOrBuild(false);
		LoadSceneFromJson(scene_to_load);
		engine->SetEditorOrBuild(engine_mode);
		scene_to_load = "";
	}
	return ret;
}

bool SceneManagerEM::PostUpdate()
{
	bool ret = true;

	ResourceManager::GetInstance().CleanUnusedResources();

	return ret;
}

bool SceneManagerEM::CleanUp()
{
	bool ret = true;
	
	if (!current_scene->CleanUp()) return false;

	return ret;
}

void SceneManagerEM::CreateEmptyScene()
{
	CleanUp();
	Awake();
}

Scene* SceneManagerEM::DuplicateScene(Scene& scene_to_copy)
{
	return new Scene(scene_to_copy);
}

void SceneManagerEM::SaveScene(std::string directory, std::string scene_name)
{
	std::string file_name_ext = scene_name + ".mlscene";
	fs::path original_file_path = fs::path(directory) / file_name_ext;
	fs::path final_file_path = original_file_path;

	if (fs::exists(original_file_path) &&
		original_file_path.string() != current_scene->GetScenePath())
	{
		fs::path parentDir = original_file_path.parent_path();
		std::string extension = original_file_path.extension().string();
		std::string originalStem = original_file_path.stem().string();

		int copyNumber = 1;
		std::string newStem;
		std::regex copyRegex(R"((.*) - Copy(?: \((\d+)\))?$)");

		do {
			std::smatch match;
			std::string currentStem = (newStem.empty()) ? originalStem : newStem;

			if (std::regex_match(currentStem, match, copyRegex)) {
				std::string baseName = match[1].str();
				if (match[2].matched) {
					copyNumber = std::stoi(match[2].str()) + 1;
				}
				else {
					copyNumber = 2;
				}
				newStem = baseName + " - Copy (" + std::to_string(copyNumber) + ")";
			}
			else {
				newStem = originalStem + " - Copy";
			}

			final_file_path = parentDir / (newStem + extension);

		} while (fs::exists(final_file_path));

		file_name_ext = final_file_path.filename().string();
	}

	fs::create_directories(fs::path(directory));

	nlohmann::json sceneJSON;

	sceneJSON["scene_name"] = current_scene->GetSceneName();
	sceneJSON["scene_id"] = current_scene->GetSceneRoot().GetID();
	sceneJSON["scene_path"] = final_file_path.string();
	current_scene->SetScenePath(final_file_path.string());

	sceneJSON["background_color"][0] = engine->renderer_em->GetBackGroundColor().r;
	sceneJSON["background_color"][1] = engine->renderer_em->GetBackGroundColor().g;
	sceneJSON["background_color"][2] = engine->renderer_em->GetBackGroundColor().b;
	sceneJSON["background_color"][3] = engine->renderer_em->GetBackGroundColor().a;

	sceneJSON["scene_gravity"] = { PhysicsEM::GetWorld()->GetGravity().x, PhysicsEM::GetWorld()->GetGravity().y };

	nlohmann::json game_objectsJSON;
	for (const auto& go : current_scene->GetSceneRoot().GetChildren())
	{
		game_objectsJSON.push_back(go->SaveGameObject());
	}
	sceneJSON["game_objects"] = game_objectsJSON;

	nlohmann::json layersJSON;
	for (const auto& layer : current_scene->GetSceneLayers())
	{
		layersJSON.push_back(layer->SaveLayer());
	}
	sceneJSON["layers"] = layersJSON;

	if (current_scene->HasCameraSet()) sceneJSON["CurrentCamID"] = current_scene->GetCurrentCameraGO().GetID();

	LOG(LogType::LOG_OK, "Succesfully Saved Scene at %s", (directory + file_name_ext).c_str());

	std::ofstream(final_file_path) << sceneJSON.dump(2);
}

void SceneManagerEM::LoadSceneFromJson(const std::string& file_name_const)
{
	std::string file_name = file_name_const;
	std::replace(file_name.begin(), file_name.end(), '/', '\\');
	if (!fs::exists(file_name))
	{
		LOG(LogType::LOG_ERROR, "Scene file does not exist: %s", file_name.c_str());
		return;
	}

	std::ifstream file(file_name);
	if (!file.is_open())
	{
		LOG(LogType::LOG_ERROR, "Failed Opening file: %s", file_name.c_str());
		return;
	}

	nlohmann::json sceneJSON;
	try
	{
		file >> sceneJSON;
	}
	catch (const nlohmann::json::parse_error& e)
	{
		LOG(LogType::LOG_ERROR, "Failed to parse scene JSON: %s", e.what());
		return;
	}
	file.close();

	if (engine->GetEngineState() == EngineState::PLAY) StopSession();

	current_scene->SetScenePath(file_name);
	if (sceneJSON.contains("scene_name"))
	{
		current_scene->SetSceneName(sceneJSON["scene_name"]);
		current_scene->GetSceneRoot().SetName(sceneJSON["scene_name"]);
	}
	if (sceneJSON.contains("scene_id")) current_scene->GetSceneRoot().SetID(sceneJSON["scene_id"]);

	if (sceneJSON.contains("background_color"))
	{
		current_scene->SetBackGroundColor({
			(int)sceneJSON["background_color"][0],
			(int)sceneJSON["background_color"][1],
			(int)sceneJSON["background_color"][2],
			(int)sceneJSON["background_color"][3],
			});
	}

	if (sceneJSON.contains("scene_gravity")) current_scene->SetSceneGravity({ sceneJSON["scene_gravity"][0], sceneJSON["scene_gravity"][1] });

	current_scene->GetSceneRoot().GetChildren().clear();

	if (sceneJSON.contains("game_objects"))
	{
		const nlohmann::json& gameObjectsJSON = sceneJSON["game_objects"];

		for (const auto& gameObjectJSON : gameObjectsJSON)
		{
			std::shared_ptr<GameObject> go = current_scene->CreateEmptyGO(current_scene->GetSceneRoot());
			go->LoadGameObject(gameObjectJSON);
		}
	}
	current_scene->GetSceneLayers().clear();

	if (sceneJSON.contains("layers"))
	{
		const nlohmann::json& layersJSON = sceneJSON["layers"];

		for (const auto& layerJSON : layersJSON)
		{
			std::shared_ptr<Layer> layer = current_scene->CreateEmptyLayer(false);
			layer->LoadLayer(layerJSON);

			if (layerJSON.contains("Layer_children_go_id"))
			{
				int children_count = 0;
				if (layerJSON.contains("children_count")) children_count = layerJSON["children_count"];
				std::vector<uint32_t> childIDs;

				for (size_t i = 0; i < children_count; i++)
				{
					childIDs.push_back(layerJSON["Layer_children_go_id"][i]);
				}

				for (auto& id : childIDs)
				{
					if (auto child = current_scene->FindGameObjectByID(id))
					{
						layer->AddChild(child);
						child->SetParentLayer(layer);
					}
					else
						LOG(LogType::LOG_WARNING, "GameObject <id: %u> not found for layer", id);
				}
			}
		}
	}
	current_scene->GetSceneRoot().CheckForEmptyLayers(current_scene.get());

	if (sceneJSON.contains("CurrentCamID")) current_scene->SetCurrentCameraGO(current_scene->FindGameObjectByID(sceneJSON["CurrentCamID"]));

	LOG(LogType::LOG_OK, "Succesfully Loaded Scene %s", file_name.c_str());

	if (engine->GetEngineState() == EngineState::PLAY)
	{
		FinishLoad();
		StartSession();
	}
}

void SceneManagerEM::LoadSceneToScene(const std::string& file_name_const, Scene& scene)
{
	std::string file_name = file_name_const;
	std::replace(file_name.begin(), file_name.end(), '/', '\\');

	if (!fs::exists(file_name))
	{
		LOG(LogType::LOG_ERROR, "Scene file does not exist: %s", file_name.c_str());
		return;
	}

	std::ifstream file(file_name);
	if (!file.is_open())
	{
		LOG(LogType::LOG_ERROR, "Failed Opening file: %s", file_name.c_str());
		return;
	}

	nlohmann::json sceneJSON;
	try
	{
		file >> sceneJSON;
	}
	catch (const nlohmann::json::parse_error& e)
	{
		LOG(LogType::LOG_ERROR, "Failed to parse scene JSON: %s", e.what());
		return;
	}
	file.close();

	bool old_log_state = engine->AreLogsStopped();
	engine->StopLogs(true);

	scene.SetScenePath(file_name);
	if (sceneJSON.contains("scene_name"))
	{
		scene.SetSceneName(sceneJSON["scene_name"]);
		scene.GetSceneRoot().SetName(sceneJSON["scene_name"]);
	}
	if (sceneJSON.contains("scene_id")) scene.GetSceneRoot().SetID(sceneJSON["scene_id"]);

	if (sceneJSON.contains("background_color"))
	{
		scene.SetBackGroundColor({
			(int)sceneJSON["background_color"][0],
			(int)sceneJSON["background_color"][1],
			(int)sceneJSON["background_color"][2],
			(int)sceneJSON["background_color"][3],
			});
	}

	if (sceneJSON.contains("scene_gravity")) scene.SetSceneGravity({ sceneJSON["scene_gravity"][0], sceneJSON["scene_gravity"][1] });

	scene.GetSceneRoot().GetChildren().clear();
	scene.GetSceneLayers().clear();

	if (sceneJSON.contains("game_objects"))
	{
		const nlohmann::json& gameObjectsJSON = sceneJSON["game_objects"];

		for (const auto& gameObjectJSON : gameObjectsJSON)
		{
			std::shared_ptr<GameObject> go = scene.CreateEmptyGO(scene.GetSceneRoot());
			go->LoadGameObject(gameObjectJSON);
		}
	}

	if (sceneJSON.contains("layers"))
	{
		const nlohmann::json& layersJSON = sceneJSON["layers"];

		for (const auto& layerJSON : layersJSON)
		{
			std::shared_ptr<Layer> layer = scene.CreateEmptyLayer(false);
			layer->LoadLayer(layerJSON);

			if (layerJSON.contains("Layer_children_go_id"))
			{
				int children_count = 0;
				if (layerJSON.contains("children_count")) children_count = layerJSON["children_count"];
				std::vector<uint32_t> childIDs;

				for (size_t i = 0; i < children_count; i++)
				{
					childIDs.push_back(layerJSON["Layer_children_go_id"][i]);
				}

				for (auto& id : childIDs)
				{
					if (auto child = scene.FindGameObjectByID(id))
					{
						layer->AddChild(child);
						child->SetParentLayer(layer);
					}
					else
						LOG(LogType::LOG_WARNING, "GameObject <id: %u> not found for layer", id);
				}
			}
		}
	}
	scene.GetSceneRoot().CheckForEmptyLayers(&scene);

	if (sceneJSON.contains("CurrentCamID")) scene.SetCurrentCameraGO(scene.FindGameObjectByID(sceneJSON["CurrentCamID"]));

	engine->StopLogs(old_log_state);
	LOG(LogType::LOG_OK, "Succesfully Loaded Scene %s", file_name.c_str());
}

void SceneManagerEM::RuntimeLoadScene(const std::string& file_name)
{
	if (scene_to_load.empty()) scene_to_load = file_name;
}

void SceneManagerEM::ImportTiledFile(const std::string& file_name_const)
{
	std::string file_name = file_name_const;
	std::replace(file_name.begin(), file_name.end(), '/', '\\');
	std::string tiledName = std::filesystem::path(file_name).stem().string();
	std::string directory_path;
	size_t last_slash_pos = file_name.find_last_of("\\");
	if (last_slash_pos != std::string::npos) {
		directory_path = file_name.substr(0, last_slash_pos);
	}
	else {
		directory_path = file_name;
	}

	if (!fs::exists(file_name))
	{
		LOG(LogType::LOG_ERROR, "Tiled file does not exist: %s", file_name.c_str());
		return;
	}

	std::ifstream file(file_name);
	if (!file.is_open())
	{
		LOG(LogType::LOG_ERROR, "Failed Opening file: %s", file_name.c_str());
		return;
	}

	nlohmann::json tiledJSON;
	try
	{
		file >> tiledJSON;
	}
	catch (const nlohmann::json::parse_error& e)
	{
		LOG(LogType::LOG_ERROR, "Failed to parse Tiled JSON: %s", e.what());
		return;
	}
	file.close();

	if (tiledJSON["orientation"] != "orthogonal" || tiledJSON["type"] != "map" || tiledJSON.count("tilesets") != 1 || tiledJSON["infinite"] != false)
	{
		LOG(LogType::LOG_ERROR, "Tiled Incompatible Format: %s\nCorrect format uses:\n - orientation: orthogonal\n - type : map\n - tilesets size must be 1\n - infinite : false\n", file_name.c_str());
		return;
	}
	if (tiledJSON["tilesets"][0].contains("source"))
	{
		LOG(LogType::LOG_ERROR, "Tiled Incompatible Format: %s\nCorrect format requires an embed TileSet to the TileMap, not an external .tsx file", file_name.c_str());
		return;
	}

	std::shared_ptr<GameObject> parent = current_scene->CreateEmptyGO(current_scene->GetSceneRoot());
	parent->SetName(tiledName);

	std::shared_ptr<Layer> container_layer = current_scene->CreateEmptyLayer();
	container_layer->SetLayerName(tiledName);

	parent->GetParentLayer().lock()->RemoveChild(parent);
	container_layer->AddChild(parent);
	parent->SetParentLayer(container_layer);

	int tile_width, tile_height, grid_width, grid_height;
	tile_width = tiledJSON["tilewidth"];
	tile_height = tiledJSON["tileheight"];
	grid_width = tiledJSON["width"];
	grid_height = tiledJSON["height"];

	int firstgid, image_section_w, image_section_h;
	firstgid = tiledJSON["tilesets"][0]["firstgid"];
	image_section_w = tiledJSON["tilesets"][0]["tilewidth"];
	image_section_h = tiledJSON["tilesets"][0]["tileheight"];
	std::string tex_path = tiledJSON["tilesets"][0]["image"];

	if (tiledJSON.contains("layers"))
	{
		const nlohmann::json& layersJSON = tiledJSON["layers"];

		for (const auto& layerJSON : layersJSON)
		{
			if (layerJSON["type"] == "tilelayer")
			{
				std::shared_ptr<GameObject> layer_go = current_scene->CreateEmptyGO(*parent);
				layer_go->SetName(layerJSON["name"]);
				layer_go->SetVisible(layerJSON["visible"]);

				layer_go->GetParentLayer().lock()->RemoveChild(layer_go);
				container_layer->AddChild(layer_go, true);
				layer_go->SetParentLayer(container_layer);

				layer_go->AddComponent<TileMap>();
				TileMap* tile_map = layer_go->GetComponent<TileMap>();

				tile_map->SwapTexture(directory_path + "\\" + tex_path);
				tile_map->ResizeGrid({ grid_width, grid_height });
				tile_map->SetImageSectionSize({ image_section_w, image_section_h });
				auto& dataArray = layerJSON["data"];
				for (size_t i = 0; i < dataArray.size(); i++)
				{
					int value = layerJSON["data"][i];
					tile_map->SetTile(tile_map->GetTile(i), value == 0 ? -1 : value - firstgid);
				}
			}
			else if (layerJSON["type"] == "objectgroup")
			{
				std::shared_ptr<GameObject> layer_go = current_scene->CreateEmptyGO(*parent);
				layer_go->SetName(layerJSON["name"]);
				bool parent_visible = layerJSON["visible"];
				layer_go->SetVisible(parent_visible);

				layer_go->GetParentLayer().lock()->RemoveChild(layer_go);
				container_layer->AddChild(layer_go, true);
				layer_go->SetParentLayer(container_layer);

				ML_Color color = ML_Color(0, 0, 0, 255);

				if (layerJSON.contains("color"))
				{
					color = ML_Color(layerJSON["color"]);
				}

				bool sensor = false;

				if (layerJSON.contains("properties"))
				{
					const nlohmann::json& propertiesJSON = layerJSON["properties"];

					for (const auto& propertyJSON : propertiesJSON)
					{
						if (propertyJSON.contains("name"))
							if (propertyJSON["name"] == "sensor")
								if (propertyJSON.contains("value"))
									sensor = propertyJSON["value"];
					}
				}
				auto& objectsArray = layerJSON["objects"];
				for (size_t i = 0; i < objectsArray.size(); i++)
				{
					const nlohmann::json& objectJSON = layerJSON["objects"][i];

					std::shared_ptr<GameObject> object_go = current_scene->CreateEmptyGO(*layer_go);

					if (objectJSON.contains("name"))
						if (objectJSON["name"] != "")
							object_go->SetName(objectJSON["name"]);
						else
							object_go->SetName("Object");
					else
						object_go->SetName("Object");

					object_go->SetVisible(parent_visible);

					object_go->GetParentLayer().lock()->RemoveChild(object_go);
					container_layer->AddChild(object_go, true);
					object_go->SetParentLayer(container_layer);

					object_go->AddComponent<Collider2D>();
					Collider2D* collider = object_go->GetComponent<Collider2D>();

					collider->SetShapeType(ShapeType::RECTANGLE);
					collider->SetDynamic(false);
					collider->SetSensor(sensor);
					collider->SetColor(color);
					ML_Rect section = { 0,0,0,0 };
					if (objectJSON.contains("x"))		section.x = PIXEL_TO_METERS(objectJSON["x"]);
					if (objectJSON.contains("y"))		section.y = PIXEL_TO_METERS(objectJSON["y"]);
					if (objectJSON.contains("width"))	section.w = PIXEL_TO_METERS(objectJSON["width"]);
					if (objectJSON.contains("height"))	section.h = PIXEL_TO_METERS(objectJSON["height"]);
					collider->SetSize(section.w == 0 ? 0.01 : section.w, section.h == 0 ? 0.01 : section.h);

					Transform* t = object_go->GetComponent<Transform>();
					t->SetPosition({ section.x + section.w / 2, -section.y - section.h / 2 });
				}
			}
		}
	}

	//
	LOG(LogType::LOG_OK, "Success Importing Tiled file <%s> from <%s>", tiledName.c_str(), file_name.c_str());
}

void SceneManagerEM::FinishLoad()
{
	if (!current_scene) return;
	current_scene->GetSceneRoot().FinishLoad();
}

void SceneManagerEM::StartSession()
{
	if (current_scene)
	{
		if (engine->GetEditorOrBuild()) // if we are in build mode, we dont need to save scene, duplicate scene, etc...
		{
			engine->StopLogs(true);
			//save scene
			if (!current_scene->GetScenePath().empty() && current_scene->GetScenePath() != "")
			{
				fs::path scene_path(current_scene->GetScenePath());

				std::string directory = scene_path.parent_path().string();
				std::string scene_name = scene_path.stem().string();

				SaveScene(directory, scene_name);
			}
			//copy the scene in memory
			pre_play_scene.reset(DuplicateScene(*current_scene.get()));
			engine->StopLogs(false);
		}
		//init all
		current_scene->Init();
	}
}

void SceneManagerEM::StopSession()
{
	if (pre_play_scene && engine->GetEditorOrBuild())
	{
		MonoRegisterer::prefab_templates.clear();
		engine->StopLogs(true);
		current_scene->CleanUp();
		PhysicsEM::Init();
		if (engine->GetEditorOrBuild()) current_scene = std::move(pre_play_scene);
		engine->StopLogs(false);
	}
	else if (!engine->GetEditorOrBuild())
	{
		current_scene->CleanUp();
	}
}

//SCENE
Scene::Scene(std::string scene_name) : scene_name(scene_name)
{
	this->scene_root = std::make_shared<GameObject>(scene_root, scene_name, true);
	this->scene_root->Awake();
	this->scene_layers.push_back(std::make_shared<Layer>(scene_layers.size(), "DefaultLayer_" + std::to_string(scene_layers.size()), true));
	LOG(LogType::LOG_INFO, "Scene <%s> created", scene_name.c_str());
}

Scene::Scene(const Scene& other)
	: scene_name(other.scene_name),
	scene_path(other.scene_path)
{
	SetBackGroundColor(other.bg_color);
	SetSceneGravity(other.world_gravity);

	this->scene_root = std::make_shared<GameObject>(other.scene_root->GetSharedPtr());
	this->scene_root->Awake();
	this->scene_root->SetID(other.scene_root->GetID());
	if (!other.scene_root->GetChildren().empty())
	{
		auto children = std::vector<std::shared_ptr<GameObject>>(other.scene_root->GetChildren());
		for (const auto& child_to_copy : children)
			DuplicateGO(*child_to_copy, true)->Reparent(this->scene_root);
	}

	for (const auto& item : other.scene_layers)
	{
		std::shared_ptr<Layer> layer = DuplicateLayer(*item);
		scene_layers.push_back(layer);
	}

	if (other.current_camera_go) current_camera_go = FindGameObjectByID(other.current_camera_go->GetID());
}

Scene::~Scene()
{
}

bool Scene::Init()
{
	for (const auto& go : scene_root->GetChildren())
		if (go->IsEnabled())
			if (!go->Init())
				return false;

	return true;
}

bool Scene::Update(double dt)
{
	bool ret = true;
	//
	AddPengindGOs();
	AddPengindLayers();
	//
	updating = true;
	if (engine->GetEngineState() == EngineState::STOP)
	{
		prefab_check_timer += (float)dt;
		if (prefab_check_timer >= prefab_check_frequency)
		{
			for (const auto& item : scene_root.get()->GetChildren())
				item->PrefabChecker();
			prefab_check_timer = 0.0f;
		}
	}

	auto childrenCopy = scene_root->GetChildren();
	auto layersCopy = scene_layers;

	for (const auto& item : childrenCopy)
		if (item->IsEnabled() && engine->GetEngineState() == EngineState::PLAY)
			if (!item->Update(dt))
				return false;

	for (const auto& item : layersCopy)
		if (item->IsVisible())
			if (!item->Update(dt))
				return false;

	updating = false;
	//
	DeletePengindGOs();
	DeletePengindLayers();
	//
	return ret;
}

bool Scene::Pause()
{
	bool ret = true;

	for (const auto& go : scene_root->GetChildren())
		if (go->IsEnabled())
			if (!go->Pause())
				return false;

	return ret;
}

bool Scene::Unpause()
{
	bool ret = true;

	for (const auto& go : scene_root->GetChildren())
		if (go->IsEnabled())
			if (!go->Unpause())
				return false;

	return ret;
}

bool Scene::CleanUp()
{
	bool ret = true;

	objects_to_add.clear();
	layers_to_add.clear();
	for (const auto& item : scene_root.get()->GetChildren())
		if (!item->CleanUp())
			return false;

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

std::shared_ptr<GameObject> Scene::DuplicateGO(GameObject& go_to_copy, bool scene_duplication, bool prefab_duplication)
{
	std::shared_ptr<GameObject> copy = std::make_shared<GameObject>(go_to_copy.GetSharedPtr());
	if (scene_duplication || prefab_duplication)
	{
		copy->SetName(go_to_copy.GetName(), false);
		if (scene_duplication) copy->SetID(go_to_copy.GetID());
	}
	copy->CloneComponents(go_to_copy.GetSharedPtr(), scene_duplication);
	if (go_to_copy.GetParentGO().lock().get())
	{
		go_to_copy.GetParentGO().lock()->AddChild(copy);
	}
	if (go_to_copy.GetParentLayer().lock() != nullptr)
	{
		ReparentToLayer(copy, go_to_copy.GetParentLayer().lock());
	}

	if (!go_to_copy.GetChildren().empty())
	{
		auto children = std::vector<std::shared_ptr<GameObject>>(go_to_copy.GetChildren());
		for (const auto& child_to_copy : children)
			DuplicateGO(*child_to_copy, scene_duplication, prefab_duplication)->Reparent(copy);
	}

	return copy;
}

std::shared_ptr<Layer> Scene::CreateEmptyLayer(bool insert_first_or_last)
{
	std::shared_ptr<Layer> empty_layer = std::make_shared<Layer>(scene_layers.size(), std::string("Layer_" + std::to_string(scene_layers.size())));
	if (insert_first_or_last)
		scene_layers.insert(scene_layers.begin(), empty_layer);
	else
		scene_layers.emplace_back(empty_layer);
	return empty_layer;
}

std::shared_ptr<Layer> Scene::DuplicateLayer(Layer& layer_to_copy)
{
	std::shared_ptr<Layer> layer = std::make_shared<Layer>(layer_to_copy);

	for (const auto& go : layer_to_copy.GetChildren())
	{
		if (auto child = FindGameObjectByID(go->GetID()))
		{
			layer->AddChild(child);
			child->SetParentLayer(layer);
		}
	}
	return layer;
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
	for (const auto& item : objects_to_add)
	{
		scene_root.get()->GetChildren().push_back(item);
		if (engine->GetEngineState() == EngineState::PLAY)
			item->Init();
	}
	objects_to_add.clear();
}

void Scene::DeletePengindGOs()
{
	for (const auto& item : objects_to_delete)
	{
		item.get()->CleanUp();
		item.get()->Delete();
	}
	objects_to_delete.clear();
}

void Scene::AddPengindLayers()
{
	for (const auto& item : layers_to_add)
	{
		scene_layers.insert(scene_layers.begin(), item);
	}
	layers_to_add.clear();
}

void Scene::DeletePengindLayers()
{
	for (const auto& item : layers_to_delete)
	{
		const size_t initial_size = scene_layers.size();

		scene_layers.erase(
			std::remove_if(scene_layers.begin(), scene_layers.end(),
				[&item](const std::shared_ptr<Layer>& c) {
					return c.get() == item.get();
				}),
			scene_layers.end()
		);
	}
	layers_to_delete.clear();
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

void Scene::ReparentToLayer(std::shared_ptr<GameObject> game_object, uint32_t target_layer_id, int position)
{
	auto current_layer = game_object->GetParentLayer().lock();
	uint32_t current_layer_id = current_layer ? current_layer->GetLayerID() : -1;
	int original_index = -1;

	if (current_layer && current_layer->GetLayerID() == target_layer_id)
		original_index = current_layer->GetGameObjectIndex(game_object);

	if (current_layer)
		current_layer->RemoveChild(game_object);

	if (auto target_layer = GetLayerByID(target_layer_id))
	{
		auto& children = target_layer->GetChildren();

		if (original_index != -1 && position > original_index)
			position--;

		position = std::clamp(position, 0, static_cast<int>(children.size()));
		children.insert(children.begin() + position, game_object);
		game_object->SetParentLayer(target_layer);
	}
}

int Scene::GetLayerIndex(uint32_t layer_id)
{
	const auto& layers = GetSceneLayers();
	for (int i = 0; i < layers.size(); ++i)
		if (layers[i]->GetLayerID() == layer_id)
			return i;

	return -1;
}

int Scene::GetGOOrderInLayer(std::shared_ptr<GameObject> go)
{
	if (!go->GetParentLayer().lock().get()) return 0;
	int layer_index = GetLayerIndex(go->GetParentLayer().lock()->GetLayerID());
	int game_object_index = go->GetParentLayer().lock()->GetGameObjectIndex(go);

	return layer_index * 10000 + game_object_index;
}

std::shared_ptr<Layer> Scene::GetLayerByID(uint32_t layer_id)
{
	auto& layers = GetSceneLayers();
	auto it = std::find_if(layers.begin(), layers.end(),
		[layer_id](const std::shared_ptr<Layer>& layer) {
			return layer->GetLayerID() == layer_id;
		});
	return (it != layers.end()) ? *it : nullptr;
}

void Scene::ReorderLayer(int old_index, uint32_t target_layer_id)
{
	auto& layers = GetSceneLayers();
	int new_index = GetLayerIndex(target_layer_id);

	if (old_index >= 0 && old_index < layers.size() &&
		new_index >= 0 && new_index < layers.size())
	{
		auto layer_to_move = layers[old_index];
		layers.erase(layers.begin() + old_index);
		layers.insert(layers.begin() + new_index, layer_to_move);
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

void Scene::DeselectGameObject(std::shared_ptr<GameObject> go)
{
	auto it = std::find_if(selected_gos.begin(), selected_gos.end(),
		[&](const auto& weak_go) { return weak_go.lock() == go; });

	if (it != selected_gos.end())
	{
		selected_gos.erase(it);

		if (last_selected == go)
		{
			if (selected_gos.empty())
			{
				last_selected = nullptr;
			}
			else
			{
				last_selected = selected_gos.back().lock();
			}
		}
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

std::shared_ptr<GameObject> Scene::FindGameObjectByID(uint32_t id)
{
	std::shared_ptr<GameObject> found = nullptr;

	TraverseHierarchy([&](std::shared_ptr<GameObject> go) {
		if (go->GetID() == id) { found = go; }
		});

	return found;
}

std::shared_ptr<GameObject> Scene::FindGameObjectByName(std::string name)
{
	std::shared_ptr<GameObject> found = nullptr;

	TraverseHierarchy([&](std::shared_ptr<GameObject> go) {
		if (go->GetName() == name) { found = go; }
		});

	return found;
}

void Scene::SetSceneName(std::string name)
{
	this->scene_name = name;
	scene_root->SetName(name);
}

ML_Color Scene::GetBackGroundColor()
{
	bg_color = engine->renderer_em->GetBackGroundColor();
	return bg_color;
}

void Scene::SetBackGroundColor(ML_Color c)
{
	bg_color = c;
	engine->renderer_em->SetBackGroundColor(bg_color);
}

vec2f Scene::GetSceneGravity()
{
	if (std::this_thread::get_id() != engine->main_thread_id) return world_gravity;
	if (PhysicsEM::GetWorld())
		world_gravity = { PhysicsEM::GetWorld()->GetGravity().x, PhysicsEM::GetWorld()->GetGravity().y };

	return world_gravity;
}

void Scene::SetSceneGravity(vec2f g)
{
	world_gravity = g;

	if (PhysicsEM::GetWorld())
	{
		PhysicsEM::GetWorld()->SetGravity({ world_gravity.x, world_gravity.y });
	}
}
