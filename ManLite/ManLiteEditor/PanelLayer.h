#ifndef __PANEL_LAYER_H__
#define __PANEL_LAYER_H__
#pragma once

#include "GuiPanel.h"

#include "unordered_map"
#include "memory"

class Layer;
class GameObject;

class PanelLayer : public Panel
{
public:
	PanelLayer(PanelType type, std::string name, bool enabled);
	~PanelLayer();

	bool Update();

	void IntroPart();
	void IterateChildren(Layer& go, bool visible);
	void BlankContext();
	void HandleLayerDragAndDrop(Layer& layer);
	void HandleGameObjectDragAndDrop(GameObject& go, Layer& target_layer);
	void GameObjectDropZone(Layer& target_layer, int position);
	void HandleLayerHeaderDrop(Layer& target_layer);

private:

	std::unordered_map<uint32_t, bool> collapsed_layers;

	bool request_uncollapse_all = false;
	bool request_collapse_all = false;

	int dragged_layer_index = -1;
	bool dragging_layer = false;

	std::weak_ptr<GameObject> dragged_gameobject;
};

#endif // !__PANEL_LAYER_H__