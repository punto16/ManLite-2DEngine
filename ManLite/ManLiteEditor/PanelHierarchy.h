#ifndef __PANEL_HIERARCHY_H__
#define __PANEL_HIERARCHY_H__
#pragma once

#include "GuiPanel.h"

class GameObject;

class PanelHierarchy : public Panel
{
public:
	PanelHierarchy(PanelType type, std::string name, bool enabled);
	~PanelHierarchy();

	bool Update();

	void IterateTree(GameObject& parent);
	void BlankContext(GameObject& parent);
	void Context(GameObject& parent);
	void DragAndDrop(GameObject& parent);
	void DropZone(GameObject& parent, int position);

private:

};

#endif // !__PANEL_HIERARCHY_H__