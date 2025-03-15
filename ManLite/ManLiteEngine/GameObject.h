#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#pragma once

#include "Component.h"
#include "Log.h"

#include <memory>
#include <vector>
#include <string>

class Layer;

class GameObject : public std::enable_shared_from_this<GameObject>
{
public:
	GameObject(std::weak_ptr<GameObject> parent, std::string name, bool enable);
	GameObject(std::weak_ptr<GameObject> go_to_copy);
	~GameObject();
	
	bool Awake();
	bool Update(double dt);

	void Draw();

	//DO NOT call this function to delete a game object
	//instead, call current_scene->SafeDeleteGO(this);
	void Delete();

	bool Reparent(std::shared_ptr<GameObject> new_parent, bool skip_descendant_search = false);
	bool IsDescendant(const std::shared_ptr<GameObject>& potential_ancestor) const;
	bool MoveInVector(int new_position);

	void CloneChildrenHierarchy(const std::shared_ptr<GameObject>& original);
	void CloneComponents(const std::shared_ptr<GameObject>& original);

	void AddChild(std::shared_ptr<GameObject> child);
	bool RemoveChild(const std::shared_ptr<GameObject>& child);
	bool RemoveChild(uint32_t id);
	bool HasChild(const std::shared_ptr<GameObject>& child) const;
	bool HasChild(uint32_t id) const;

	//util
	static uint32_t GenerateGameObjectID();
	static std::string GenerateUniqueName(const std::string& baseName, const GameObject* go);
	std::shared_ptr<GameObject> GetSharedPtr() { return shared_from_this(); }

	//components
	//get the component selected
	template <typename TComponent>
	TComponent* GetComponent()
	{
		for (const auto& component : components_gameobject)
		{
			if (dynamic_cast<TComponent*>(component.get()))
				return static_cast<TComponent*>(component.get());
		}
		return nullptr;
	}

	//get all components available of the same type (useful for scripts)
	template <typename TComponent>
	std::vector<TComponent*> GetComponents() const
	{
		std::vector<TComponent*> matchingComponents;
		for (const auto& component : components_gameobject)
		{
			if (TComponent* castedComponent = dynamic_cast<TComponent*>(component.get())) {
				matchingComponents.push_back(castedComponent);
			}
		}
		return matchingComponents;
	}

	template <typename TComponent, typename... Args>
	bool AddComponent(Args&&... args)
	{
		Component* component = this->GetComponent<TComponent>();

		//check if already exists
		if (component && component->GetType() != ComponentType::Script)
		{
			LOG(LogType::LOG_WARNING, "Component already applied");
			LOG(LogType::LOG_INFO, "-GameObject [Name: %s] ", this->gameobject_name.c_str());
			LOG(LogType::LOG_INFO, "-Component  [Type: %s] ", component->GetName().c_str());
			return false;
		}
		std::unique_ptr<Component> new_component = std::make_unique<TComponent>(shared_from_this(), std::forward<Args>(args)...);
		components_gameobject.push_back(std::move(new_component));

		return true;
	}

	template <typename TComponent>
	bool AddCopiedComponent(const TComponent& ref)
	{
		Component* component = this->GetComponent<TComponent>();

		//check if already exists
		if (component && component->GetType() != ComponentType::Script)
		{
			LOG(LogType::LOG_WARNING, "Component already applied");
			LOG(LogType::LOG_INFO, "-GameObject [Name: %s] ", this->gameobject_name.c_str());
			LOG(LogType::LOG_INFO, "-Component  [Type: %s] ", component->GetName().c_str());

			return false;
		}

		std::unique_ptr<Component> newComponent = std::make_unique<TComponent>(ref, shared_from_this());
		components_gameobject.push_back(std::move(newComponent));

		return true;
	}

	//remove component by its type //better not use for scripts
	void RemoveComponent(ComponentType type)
	{
		for (auto it = components_gameobject.begin(); it != components_gameobject.end(); ++it)
		{
			if ((*it)->GetType() == type)
			{
				it = components_gameobject.erase(it);
				break;
			}
		}
	}

	//remove component by pointer
	template <typename TComponent>
	void RemoveComponent(TComponent* compToRemove)
	{
		auto it = std::find_if(components_gameobject.begin(), components_gameobject.end(),
			[compToRemove](const std::unique_ptr<Component>& comp) {
				return comp.get() == compToRemove;
			});

		if (it != components_gameobject.end())
			components_gameobject.erase(it);
	}

	//getters // setters
	std::string GetName() const { return this->gameobject_name; }
	void SetName(std::string name) { this->gameobject_name = GenerateUniqueName(name, this); }
	std::string GetTag() const { return this->gameobject_tag; }
	void SetTag(std::string tag) { this->gameobject_tag = tag; }
	uint32_t GetID() const { return this->gameobject_id; }
	std::weak_ptr<GameObject> GetParentGO() const { return this->parent_gameobject; }
	std::weak_ptr<Layer> GetParentLayer() const { return this->parent_layer; }
	void SetParentLayer(std::shared_ptr<Layer> layer) { this->parent_layer = layer; }
	std::vector<std::shared_ptr<GameObject>>& GetChildren() { return children_gameobject; }
	std::vector<std::unique_ptr<Component>>& GetComponents() { return components_gameobject; }
	bool IsVisible() const { return this->visible; }
	void SetVisible(bool visible) { this->visible = visible; }
	void SwitchVisible() { SetVisible(!visible); }
	bool IsEnabled() const { return this->enabled; }
	void SetEnabled(bool enable)
	{
		if (this->enabled && !enable)
		{
			//disable go, components BUT NOT THE children
			this->enabled = false;
			for (const auto& component : components_gameobject) component->SetEnabled(false);
		}
		else if (!this->enabled && enable)
		{
			//enable go, components BUT NOT THE children
			this->enabled = true;
			for (const auto& component : components_gameobject) component->SetEnabled(true);
		}
	}
	void SwitchEnabled() { SetEnabled(!this->enabled); }

private:
	std::string gameobject_name;
	std::string gameobject_tag;
	uint32_t gameobject_id;

	std::weak_ptr<GameObject> parent_gameobject;
	std::weak_ptr<Layer> parent_layer;
	std::vector<std::shared_ptr<GameObject>> children_gameobject;
	std::vector<std::unique_ptr<Component>> components_gameobject;

	bool enabled;
	bool visible;
};

#endif // !__GAMEOBJECT_H__