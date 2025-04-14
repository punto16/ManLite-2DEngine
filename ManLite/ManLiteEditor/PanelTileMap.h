#ifndef __PANEL_TILEMAP_H__
#define __PANEL_TILEMAP_H__
#pragma once

#include "GuiPanel.h"

#include "imgui.h"
#include "Defs.h"
#include <set>
#include <algorithm>

class TileMap;

class PanelTileMap : public Panel
{
public:
	PanelTileMap(PanelType type, std::string name, bool enabled);
	~PanelTileMap();

	bool Update();

	void SetMap(TileMap* tilemap);

private:
	TileMap* tilemap;

	ImVec2 canvas_pos;
	ImVec2 canvas_size;
	ImVec2 texture_size;
	ImVec2 hovered_tile{ -1, -1 };
	int selected_tile_id = -1;
	vec2 selected_grid_tile = { -1, -1 };

	std::set<std::pair<int, int>> selected_tiles;
	bool is_selecting = false;
	vec2 selection_start = { -1, -1 };
	vec2 selection_end = { -1, -1 };

	bool ctrl_pressed_during_click = false;
};

#endif // !__PANEL_TILEMAP_H__