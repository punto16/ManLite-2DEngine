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
	~Scene();

	bool Update(double dt);

	bool CleanUp();

	//general functions for game objects
	std::shared_ptr<GameObject> CreateEmptyGO(GameObject& parent);
	std::shared_ptr<GameObject> DuplicateGO(GameObject& go_to_copy);
	//general functions for layers
	std::shared_ptr<Layer> CreateEmptyLayer();

	//safely adds a game object to a pending to add list
	//whenever it is safe place, they will be added to scene root go
	void SafeAddGO(std::shared_ptr<GameObject> object_to_add);

	//safely deletes a game object to a pending to add list
	//whenever it is safe place, they will be deleted scene
	void SafeDeleteGO(std::shared_ptr<GameObject> object_to_delete);

	//safely adds a layer to a pending to add list
	//whenever it is safe place, they will be added to scene
	void SafeAddLayer(std::shared_ptr<Layer> layer_to_add);

	//safely deletes a layer to a pending to add list
	//whenever it is safe place, they will be deleted scene
	void SafeDeleteLayer(std::shared_ptr<Layer> layer_to_delete);

	//this function will create game objects in a safe way
	void AddPengindGOs();

	//this function will delete game objects in a safe way
	void DeletePengindGOs();

	//this function will create layers in a safe way
	void AddPengindLayers();

	//this function will delete layers in a safe way
	void DeletePengindLayers();

	//
	void ReparentToLayer(std::shared_ptr<GameObject> game_object, std::shared_ptr<Layer> target_layer);
	bool ContainsLayer(const std::shared_ptr<Layer>& layer) const;
	void SelectGameObject(std::shared_ptr<GameObject> go, bool additive = false, bool unselect_all = false);
	void SelectRange(std::shared_ptr<GameObject> endGO);
	void TraverseRecursive(std::shared_ptr<GameObject> go, const std::function<void(std::shared_ptr<GameObject>)>& func);
	void TraverseHierarchy(const std::function<void(std::shared_ptr<GameObject>)>& func)
	{
		TraverseRecursive(scene_root, func);
	}

	//getters // setters
	GameObject& GetSceneRoot() const { return *scene_root; }
	const std::vector<std::shared_ptr<Layer>>& GetSceneLayers() const { return scene_layers; }
	const std::vector<std::weak_ptr<GameObject>>& GetSelectedGOs() const { return selected_gos; }

	std::string GetSceneName() const { return this->scene_name; }
	void SetSceneName(std::string name);

private:
	std::string scene_name;
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

	Scene& GetCurrentScene() const { return *current_scene; }

private:
	std::unique_ptr<Scene> current_scene;
};

#endif // !__SCENEMANAGER_EM_H__