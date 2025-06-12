#include "Layer.h"

#include "GameObject.h"
#include "Defs.h"
#include "algorithm"

Layer::Layer(uint32_t layer_id, std::string layer_name, bool visible) :
	layer_id(layer_id),
	layer_name(layer_name == "Layer" ? layer_name + std::to_string(layer_id) : layer_name),
	visible(visible)
{
}

Layer::Layer(const Layer& other)
	: layer_id(other.layer_id),
	layer_name(other.layer_name),
	visible(other.visible),
	children_gameobject()
{
}

Layer::~Layer()
{
}

bool Layer::Update(double dt)
{
	bool ret = true;

	auto children_copy = children_gameobject;
	std::ranges::reverse(children_copy);

	for (const auto& item : children_copy)
		if (item.get())
			if (item->IsVisible())
				item->Draw();

	return ret;
}

void Layer::AddChild(std::shared_ptr<GameObject> child, bool add_to_front)
{
	if (add_to_front)
	{
		children_gameobject.insert(children_gameobject.begin(), child);
	}
	else
	{
		children_gameobject.push_back(child);
	}
}

void Layer::AddChild(std::shared_ptr<GameObject> child, std::shared_ptr<GameObject> reference, bool add_to_front)
{
	auto it = std::find(children_gameobject.begin(), children_gameobject.end(), reference);

	if (it != children_gameobject.end())
	{
		if (add_to_front)
		{
			children_gameobject.insert(it, child);
		}
		else
		{
			children_gameobject.insert(it + 1, child);
		}
	}
	else
	{
		AddChild(child, add_to_front);
	}
}

bool Layer::RemoveChild(const std::shared_ptr<GameObject>& child)
{
	const size_t initial_size = children_gameobject.size();

	children_gameobject.erase(
		std::remove_if(children_gameobject.begin(), children_gameobject.end(),
			[&child](const std::shared_ptr<GameObject>& c) {
				return c.get() == child.get();
			}),
		children_gameobject.end()
	);

	return initial_size != children_gameobject.size();
}

bool Layer::RemoveChild(uint32_t id)
{
	const size_t initial_size = children_gameobject.size();

	children_gameobject.erase(
		std::remove_if(children_gameobject.begin(), children_gameobject.end(),
			[id](const std::shared_ptr<GameObject>& c) {
				return c->GetID() == id;
			}),
		children_gameobject.end()
	);

	return initial_size != children_gameobject.size();
}

bool Layer::HasChild(const std::shared_ptr<GameObject>& child) const
{
	return std::any_of(children_gameobject.begin(), children_gameobject.end(),
		[&child](const std::shared_ptr<GameObject>& c) {
			return c.get() == child.get();
		});
}

bool Layer::HasChild(uint32_t id) const
{
	return std::any_of(children_gameobject.begin(), children_gameobject.end(),
		[id](const std::shared_ptr<GameObject>& c) {
			return c->GetID() == id;
		});
}

nlohmann::json Layer::SaveLayer()
{
	nlohmann::json layerJSON;

	layerJSON["layer_id"] = layer_id;
	layerJSON["layer_name"] = layer_name;
	layerJSON["visible"] = visible;

	layerJSON["children_count"] = children_gameobject.size();
	int i = 0;
	for (const auto& child : children_gameobject)
	{
		layerJSON["Layer_children_go_id"][i] = child->GetID();
		i++;
	}

	return layerJSON;
}

void Layer::LoadLayer(const nlohmann::json& layerJSON)
{
	if (layerJSON.contains("layer_id")) layer_id = layerJSON["layer_id"];
	if (layerJSON.contains("layer_name")) layer_name = layerJSON["layer_name"];
	if (layerJSON.contains("visible")) visible = layerJSON["visible"];
}
