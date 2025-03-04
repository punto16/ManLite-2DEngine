#ifndef __SCENEMANAGER_EM_H__
#define __SCENEMANAGER_EM_H__
#pragma once

#include "EngineModule.h"
#include <string>
#include <memory>
#include <vector>

class GameObject;
class Layer;

class Scene
{
public:
	Scene(std::string scene_name = "Untitled Scene");
	~Scene();

	bool Update(double dt);

	//safely adds a game object to a pending to add list
	//whenever it is safe place, they will be added to scene
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

private:
	std::string scene_name;
	//scene_root because gameobjects are tree-like structure
	std::shared_ptr<GameObject> scene_root;
	//vector of Layer because they dont work as a tree
	//scene can contain layers
	//layers can ONLY contain game object (that are going to render)
	//layers can NOT contain layers
	std::vector<std::shared_ptr<Layer>> scene_layers;

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

private:
	std::unique_ptr<Scene> current_scene;
};

#endif // !__SCENEMANAGER_EM_H__