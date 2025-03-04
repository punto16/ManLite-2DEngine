#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#pragma once

#include <memory>
#include <vector>
#include <string>

class GameObject : public std::enable_shared_from_this<GameObject>
{
public:
	GameObject(std::weak_ptr<GameObject> parent, std::string name, bool enable);
	GameObject(std::weak_ptr<GameObject> go_to_copy);
	~GameObject();
	
	//DO NOT call this function to delete a game object
	//instead, call current_scene->SafeDeleteGO(this);
	void Delete();

	bool Reparent(std::shared_ptr<GameObject> new_parent);
	bool IsDescendant(const std::shared_ptr<GameObject>& potential_ancestor) const;

	void CloneChildrenHierarchy(const std::shared_ptr<GameObject>& original);

	void AddChild(std::shared_ptr<GameObject> child);
	bool RemoveChild(const std::shared_ptr<GameObject>& child);
	bool RemoveChild(uint32_t id);
	bool HasChild(const std::shared_ptr<GameObject>& child) const;
	bool HasChild(uint32_t id) const;

	uint32_t GenerateGameObjectID();

	//getters // setters
	std::string GetName() const { return this->gameobject_name; }
	void SetName(std::string name) { this->gameobject_name = name; }
	uint32_t GetID() const { return this->gameobject_id; }
	std::weak_ptr<GameObject> GetParentGO() const { return this->parent_gameobject; }
	//std::weak_ptr<Layer> GetParentLayer() const { return this->parent_layer; }
	std::vector<std::shared_ptr<GameObject>>& GetChildren() { return children_gameobject; }

	bool IsEnabled() const { return this->enabled; }
	void SetEnabled(bool enable)
	{
		if (this->enabled)
		{
			this->enabled = false;
			//disable go, components BUT NOT THE children
		}
		else if (!this->enabled)
		{
			this->enabled = true;
			//enable go, components BUT NOT THE children
		}
	}
	void SwitchEnabled() { SetEnabled(!this->enabled); }

private:
	std::string gameobject_name;
	uint32_t gameobject_id;

	std::weak_ptr<GameObject> parent_gameobject;
	//std::weak_ptr<Layer> parent_layer;
	std::vector<std::shared_ptr<GameObject>> children_gameobject;

	bool enabled;
};

#endif // !__GAMEOBJECT_H__