#ifndef __LAYER_H__
#define __LAYER_H__
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

class GameObject;
class json;

class Layer : public std::enable_shared_from_this<Layer>
{
public:
	Layer(uint32_t layer_id = -1, std::string layer_name = "Layer", bool visible = true);
	Layer(const Layer& other);
	~Layer();

	bool Update(double dt);

	//utils
	void AddChild(std::shared_ptr<GameObject> child, bool add_to_front = false);
	bool RemoveChild(const std::shared_ptr<GameObject>& child);
	bool RemoveChild(uint32_t id);
	bool HasChild(const std::shared_ptr<GameObject>& child) const;
	bool HasChild(uint32_t id) const;

	//serialization
	nlohmann::json SaveLayer();
	void LoadLayer(const nlohmann::json& layerJSON);

	//getters // setters
	int GetGameObjectIndex(const std::shared_ptr<GameObject>& go) const {
		auto it = std::find(children_gameobject.begin(), children_gameobject.end(), go);
		return (it != children_gameobject.end()) ? std::distance(children_gameobject.begin(), it) : -1;
	}
	uint32_t GetLayerID() const { return this->layer_id; }
	void SetLayerID(uint32_t id) { this->layer_id = id; }
	std::string GetLayerName() const { return this->layer_name; }
	void SetLayerName(std::string name) { this->layer_name = name; }
	bool IsVisible() const { return this->visible; }
	void SetVisible(bool visible) { this->visible = visible; }
	void SwitchVisible() { SetVisible(!visible); }
	std::vector<std::shared_ptr<GameObject>>& GetChildren() { return children_gameobject; }

private:
	//id in layers start from 1, 2, 3 ...
	uint32_t layer_id;
	std::string layer_name;
	bool visible;

	std::vector<std::shared_ptr<GameObject>> children_gameobject;

};

#endif // !__LAYER_H__