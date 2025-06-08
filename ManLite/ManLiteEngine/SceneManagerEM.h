#ifndef __SCENEMANAGER_EM_H__
#define __SCENEMANAGER_EM_H__
#pragma once

#include "EngineModule.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>

class GameObject;
class Layer;

class Scene
{
public:
	Scene(std::string scene_name = "Untitled_Scene");
	Scene(const Scene& other);
	~Scene();

	bool Init();
	bool Update(double dt);

	bool Pause();
	bool Unpause();

	bool CleanUp();

	//general functions for game objects
	std::shared_ptr<GameObject> CreateEmptyGO(GameObject& parent);
	std::shared_ptr<GameObject> DuplicateGO(GameObject& go_to_copy, bool scene_duplication = false);
	//general functions for layers
	std::shared_ptr<Layer> CreateEmptyLayer(bool insert_first_or_last = true);
	std::shared_ptr<Layer> DuplicateLayer(Layer& layer_to_copy);

	//safely adds a game object to a pending to delete list
	//whenever it is safe place, they will be added to scene root go
	void SafeAddGO(std::shared_ptr<GameObject> object_to_add);

	//safely adds a game object to a pending to add list
	//whenever it is safe place, they will be deleted scene
	void SafeDeleteGO(std::shared_ptr<GameObject> object_to_delete);

	//safely adds a layer to a pending to add list
	//whenever it is safe place, they will be added to scene
	void SafeAddLayer(std::shared_ptr<Layer> layer_to_add);

	//safely adds a layer to a pending to delete list
	//whenever it is safe place, they will be deleted scene
	void SafeDeleteLayer(std::shared_ptr<Layer> layer_to_delete);

private:
	//this function will create game objects in a safe way
	void AddPengindGOs();

	//this function will delete game objects in a safe way
	void DeletePengindGOs();

	//this function will create layers in a safe way
	void AddPengindLayers();

	//this function will delete layers in a safe way
	void DeletePengindLayers();
public:
	//
	void ReparentToLayer(std::shared_ptr<GameObject> game_object, std::shared_ptr<Layer> target_layer);
	void ReparentToLayer(std::shared_ptr<GameObject> game_object, uint32_t target_layer_id, int position);
	int GetLayerIndex(uint32_t layer_id);
	int GetGOOrderInLayer(std::shared_ptr<GameObject> go);
	std::shared_ptr<Layer> GetLayerByID(uint32_t layer_id);
	void ReorderLayer(int old_index, uint32_t target_layer_id);
	bool ContainsLayer(const std::shared_ptr<Layer>& layer) const;
	//
	void SelectGameObject(std::shared_ptr<GameObject> go, bool additive = false, bool unselect_all = false);
	void DeselectGameObject(std::shared_ptr<GameObject> go);
	void SelectRange(std::shared_ptr<GameObject> endGO);
	void TraverseRecursive(std::shared_ptr<GameObject> go, const std::function<void(std::shared_ptr<GameObject>)>& func);
	void TraverseHierarchy(const std::function<void(std::shared_ptr<GameObject>)>& func)
	{
		TraverseRecursive(scene_root, func);
	}
	std::shared_ptr<GameObject> FindGameObjectByID(uint32_t id);
	std::shared_ptr<GameObject> FindGameObjectByName(std::string name);


	//getters // setters
	GameObject& GetSceneRoot() const { return *scene_root; }
	std::vector<std::shared_ptr<Layer>>& GetSceneLayers() { return scene_layers; }
	const std::vector<std::weak_ptr<GameObject>>& GetSelectedGOs() const { return selected_gos; }
	void UnselectAll() { selected_gos.clear(); }

	std::string GetSceneName() const { return this->scene_name; }
	void SetSceneName(std::string name);
	std::string GetScenePath() const { return this->scene_path; }
	void SetScenePath(std::string scene_path) { this->scene_path = scene_path; }

	GameObject& GetCurrentCameraGO() const { return *current_camera_go; }
	bool HasCameraSet() { return current_camera_go ? true : false; }
	void SetCurrentCameraGO(std::shared_ptr<GameObject> new_cam_go) { this->current_camera_go = new_cam_go; }

private:
	std::string scene_name;
	std::string scene_path;
	//scene_root because gameobjects are tree-like structure
	std::shared_ptr<GameObject> scene_root;
	//vector of Layer because they dont work as a tree
	//scene can contain layers
	//layers can ONLY contain game object (that are going to render)
	//layers can NOT contain layers
	std::vector<std::shared_ptr<Layer>> scene_layers;
	std::vector<std::weak_ptr<GameObject>> selected_gos;
	std::shared_ptr<GameObject> last_selected = nullptr;

	std::vector<std::shared_ptr<GameObject>> objects_to_add;
	std::vector<std::shared_ptr<GameObject>> objects_to_delete;
	std::vector<std::shared_ptr<Layer>> layers_to_add;
	std::vector<std::shared_ptr<Layer>> layers_to_delete;

	std::shared_ptr<GameObject> current_camera_go = nullptr;

	float prefab_check_timer = 0.0f;
	float prefab_check_frequency = 1.5;
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

	void CreateEmptyScene();
	Scene* DuplicateScene(Scene& scene_to_copy);

	//scene serialization
	void SaveScene(std::string directory, std::string scene_name);
	void LoadSceneFromJson(const std::string& file_name);
	void LoadSceneToScene(const std::string& file_name, Scene& scene);
	void RuntimeLoadScene(const std::string& file_name);
	void ImportTiledFile(const std::string& file_name);


	void FinishLoad();


	//engine state scene manager
	void StartSession();
	void StopSession();

	//getters // setters
	Scene& GetCurrentScene() { return *current_scene; }

	bool CurrentSceneAvailable() { return current_scene.get(); }

private:
	std::unique_ptr<Scene> current_scene;

	std::unique_ptr<Scene> pre_play_scene;

	std::string scene_to_load = "";
};

#endif // !__SCENEMANAGER_EM_H__