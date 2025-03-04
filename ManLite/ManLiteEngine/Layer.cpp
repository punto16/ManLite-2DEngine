#include "Layer.h"

#include "GameObject.h"

Layer::Layer(uint32_t layer_id, std::string layer_name, bool visible) :
	layer_id(layer_id),
	layer_name(layer_name == "Layer" ? layer_name + std::to_string(layer_id) : layer_name),
	visible(visible)
{
}

Layer::~Layer()
{
}

bool Layer::Update(double dt)
{
	bool ret = true;

	for (const auto& item : children_gameobject) item->Draw();

	return ret;
}
