#ifndef __PANEL_TILEMAP_H__
#define __PANEL_TILEMAP_H__
#pragma once

#include "GuiPanel.h"

#include "imgui.h"
#include "Defs.h"
#include <set>
#include <algorithm>

class TileMap;

enum class PaintType 
{
	PAINT_BY_SELECTION,
	SELECT_AND_PAINT
};

class PanelTileMap : public Panel
{
public:
	PanelTileMap(PanelType type, std::string name, bool enabled);
	~PanelTileMap();

	bool Update();

	void UpdateBrushTiles();
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
	bool show_numbers = true;

	float zoom_level = 1.0f;
	const float min_zoom = 0.2f;
	const float max_zoom = 4.0f;

	PaintType paint_type = PaintType::PAINT_BY_SELECTION;
	vec2f texture_selection_start;
	vec2f texture_selection_end;
	bool is_texture_selecting = false;
	std::vector<int> brush_tiles; // Tiles del pincel (IDs)
	vec2f brush_size = { 1,1 };
};

#endif // !__PANEL_TILEMAP_H__