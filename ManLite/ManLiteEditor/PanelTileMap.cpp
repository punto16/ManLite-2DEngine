#include "PanelTileMap.h"

#include "GUI.h"
#include "ResourceManager.h"
#include "TileMap.h"
#include "Log.h"
#include "Canvas.h"
#include "GameObject.h"

#include "EngineCore.h"
#include "InputEM.h"

#include <imgui.h>
#include "imgui_internal.h"
#include "algorithm"

PanelTileMap::PanelTileMap(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelTileMap::~PanelTileMap()
{
}

bool PanelTileMap::Update()
{
    if (bringToFront)
    {
        ImGui::SetNextWindowFocus();
        bringToFront = false;
    }

    bool ret = true;
    auto map = tilemap;

    if (ImGui::Begin(name.c_str(), &enabled, ImGuiWindowFlags_MenuBar))
    {
        if (!map) {
            ImGui::Text("No TileMap selected");
            ImGui::End();
            return true;
        }
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("Editing TileMap of game object <%s - id: %u>", map->GetContainerGO()->GetName().c_str(), map->GetContainerGO()->GetID());
            ImGui::EndMenuBar();
        }
        HandleShortCuts();

        ImGui::Columns(2, "TileMapEditor", true);

        ImGui::BeginChild("TilesetPanel", ImVec2(0, 0), true);
        {
            int tex_w, tex_h;
            map->GetTextureSize(tex_w, tex_h);
            texture_size = ImVec2(static_cast<float>(tex_w), static_cast<float>(tex_h));

            const float avail_width = ImGui::GetContentRegionAvail().x;
            const float scale = std::min(avail_width / texture_size.x, 1.0f);
            canvas_size = ImVec2(texture_size.x * scale, texture_size.y * scale);

            ImGui::Image(
                (ImTextureID)map->GetTextureID(),
                canvas_size,
                ImVec2(0, 1),
                ImVec2(1, 0)
            );

            canvas_pos = ImGui::GetItemRectMin();

            if (ImGui::IsItemHovered() && map->GetImageSectionSize().x > 0 && map->GetImageSectionSize().y > 0)
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                const float tile_w = map->GetImageSectionSize().x * scale;
                const float tile_h = map->GetImageSectionSize().y * scale;
                const int columns = static_cast<int>(texture_size.x / map->GetImageSectionSize().x);
                const int rows = static_cast<int>(texture_size.y / map->GetImageSectionSize().y);

                for (int x = 0; x <= columns; ++x) {
                    const float x_pos = canvas_pos.x + x * tile_w;
                    draw_list->AddLine(ImVec2(x_pos, canvas_pos.y), ImVec2(x_pos, canvas_pos.y + canvas_size.y), IM_COL32(255, 255, 255, 100));
                }
                for (int y = 0; y <= rows; ++y) {
                    const float y_pos = canvas_pos.y + y * tile_h;
                    draw_list->AddLine(ImVec2(canvas_pos.x, y_pos), ImVec2(canvas_pos.x + canvas_size.x, y_pos), IM_COL32(255, 255, 255, 100));
                }

                    const ImVec2 mouse_pos = ImGui::GetMousePos();
                if (paint_type == PaintType::SELECT_AND_PAINT)
                {
                    const float rel_x = (mouse_pos.x - canvas_pos.x) / scale;
                    const float rel_y = (mouse_pos.y - canvas_pos.y) / scale;

                    if (ImGui::IsMouseClicked(0))
                    {
                        is_texture_selecting = true;
                        texture_selection_start = {
                            static_cast<int>(rel_x / map->GetImageSectionSize().x),
                            static_cast<int>(rel_y / map->GetImageSectionSize().y)
                        };
                        texture_selection_end = texture_selection_start;
                    }

                    if (is_texture_selecting)
                    {
                        texture_selection_end = {
                            static_cast<int>(rel_x / map->GetImageSectionSize().x),
                            static_cast<int>(rel_y / map->GetImageSectionSize().y)
                        };

                        ImVec2 start_pos = {
                            canvas_pos.x + texture_selection_start.x * (float)map->GetImageSectionSize().x * scale,
                            canvas_pos.y + texture_selection_start.y * (float)map->GetImageSectionSize().y * scale
                        };
                        ImVec2 end_pos = {
                            canvas_pos.x + (texture_selection_end.x + 1) * (float)map->GetImageSectionSize().x * scale,
                            canvas_pos.y + (texture_selection_end.y + 1) * (float)map->GetImageSectionSize().y * scale
                        };
                        draw_list->AddRect(start_pos, end_pos, IM_COL32(255, 0, 0, 255), 0.0f, 0, 2.0f);
                    }

                    if (ImGui::IsMouseReleased(0))
                    {
                        is_texture_selecting = false;
                        UpdateBrushTiles();
                    }
                }
                else
                {
                    if (ImGui::IsMouseClicked(0))
                    {
                        const float rel_x = (mouse_pos.x - canvas_pos.x) / scale;
                        const float rel_y = (mouse_pos.y - canvas_pos.y) / scale;
                        selected_tile_id = static_cast<int>(rel_x / map->GetImageSectionSize().x) +
                            static_cast<int>(rel_y / map->GetImageSectionSize().y) * columns;

                        if (!selected_tiles.empty()) {
                            for (const auto& [x, y] : selected_tiles) {
                                map->SetTile({ x, y }, selected_tile_id);
                            }
                        }
                    }
                }
            }
        }
        ImGui::EndChild();

        ImGui::NextColumn();
        ImGui::BeginChild("PropertiesPanel", ImVec2(0, 0), true);
        {
            ImGui::Text("Paint Mode:");
            ImGui::SameLine();
            Gui::HelpMarker("Instructions:\n\n- Selection (Ctrl + M): Allows you to select tiles in your Grid Preview and then\n   select a tile from the TileSet and it will paint all selected tiles of the grid from that\n   tile\n- Brush (Ctrl + B): Allows you to select one or tile or a section of tiles from the\n   TileSet and paint your Grid Preview using the selected tiles as a brush\n- Bucket (Ctrl + G): Allows you to select one tile of the TileSet and bucket fill your\n   Grid Preview");
            ImGui::SameLine();
            Gui::HelpMarker("Left Click to Paint/Select\nRight Click to Erase\nCtrl + U to Deselect");
            int paint_mode = static_cast<int>(paint_type);
            if (ImGui::RadioButton("Selection", &paint_mode, 0)) {
                paint_type = PaintType::PAINT_BY_SELECTION;
                selected_tiles.clear();
                brush_tiles.clear();
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Brush", &paint_mode, 1)) {
                paint_type = PaintType::SELECT_AND_PAINT;
                selected_tiles.clear();
                brush_tiles.clear();
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Bucket", &paint_mode, 2)) {
                paint_type = PaintType::BUCKET_PAINT;
                selected_tiles.clear();
                brush_tiles.clear();
            }

            if (ImGui::CollapsingHeader("Tile Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                vec2f section_size = map->GetImageSectionSize();
                if (ImGui::DragFloat2("Tile Size in Texture", &section_size.x, 1.0f, 1.0f, 2048.0f)) {
                    map->SetImageSectionSize(section_size);
                }

                vec2f grid_size = map->GetGridSize();
                if (ImGui::DragFloat2("Grid Dimensions", &grid_size.x, 1.0f, 1.0f, 256.0f)) {
                    map->ResizeGrid(grid_size, -1);
                }

                vec2f world_tile_size = map->GetTileSize();
                if (ImGui::DragFloat2("World Tile Size", &world_tile_size.x, 0.1f, 0.1f, 10.0f)) {
                    map->SetTileSize(world_tile_size);
                }

                ImGui::Separator();
                if (paint_type == PaintType::PAINT_BY_SELECTION) {
                    ImGui::Text("Selected Tile: %d", selected_tile_id);
                }
                else {
                    ImGui::Text("Brush Size: %dx%d", brush_size.x, brush_size.y);
                }

                if (selected_tile_id >= 0 || !brush_tiles.empty()) {
                    int tex_w, tex_h;
                    map->GetTextureSize(tex_w, tex_h);
                    int tiles_per_row = tex_w / map->GetImageSectionSize().x;

                    if (paint_type == PaintType::PAINT_BY_SELECTION) {
                        ImGui::Text("Tile Position: (%d, %d)",
                            selected_tile_id % tiles_per_row,
                            selected_tile_id / tiles_per_row);
                    }
                    else {
                        ImGui::Text("Brush Region: (%d,%d) to (%d,%d)",
                            texture_selection_start.x, texture_selection_start.y,
                            texture_selection_end.x, texture_selection_end.y);
                    }
                }
            }

            if (ImGui::CollapsingHeader("Grid Preview", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const float available_width = ImGui::GetContentRegionAvail().x;
                const float available_height = ImGui::GetContentRegionAvail().y - 80;

                ImGui::Checkbox("Show Numbers", &show_numbers);
                ImGui::SameLine();
                ImGui::Dummy({ 5, 0 });
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100);
                ImGui::DragFloat("Zoom Level", &zoom_level, 0.1f, 0.1f, 10.0f, "%.1f");
                ImGui::SameLine();
                if (ImGui::Button("Reset")) {
                    zoom_level = 1.0f;
                    ImGui::SetScrollX(0);
                    ImGui::SetScrollY(0);
                }

                if (ImGui::GetIO().KeyCtrl)
                {
                    float scroll_y = engine->input_em->GetMouseWheelMotion();
                    zoom_level = CLAMP(zoom_level + 0.1 * scroll_y, 10, 0.1);
                }

                ImGui::BeginChild("GridPreview", ImVec2(available_width, available_height), true,
                    ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                const ImVec2 p = ImGui::GetCursorScreenPos();
                const ImVec2 mouse_pos = ImGui::GetMousePos();

                const float base_cell_size = 32.0f;
                const float cell_size = base_cell_size * zoom_level;

                const float total_width = cell_size * map->GetGridSize().x;
                const float total_height = cell_size * map->GetGridSize().y;

                ImGui::SetCursorPos(ImVec2(0, 0));
                ImGui::Dummy(ImVec2(total_width, total_height));

                const float scroll_x = ImGui::GetScrollX();
                const float scroll_y = ImGui::GetScrollY();

                auto pos_to_grid = [&](const ImVec2& pos) -> vec2 {
                    const float grid_x = (pos.x - p.x + scroll_x) / cell_size;
                    const float grid_y = (pos.y - p.y + scroll_y) / cell_size;

                    return {
                        static_cast<int>(std::floor(grid_x)),
                        static_cast<int>(std::floor(grid_y))
                    };
                    };

                if (ImGui::IsWindowHovered()) {
                    if (paint_type == PaintType::BUCKET_PAINT && ImGui::IsMouseClicked(0)) {
                        vec2 grid_pos = pos_to_grid(mouse_pos);
                        grid_pos.x = CLAMP(grid_pos.x, static_cast<int>(map->GetGridSize().x) - 1, 0);
                        grid_pos.y = CLAMP(grid_pos.y, static_cast<int>(map->GetGridSize().y) - 1, 0);

                        if (selected_tile_id != -1) {
                            BucketFill(grid_pos, selected_tile_id);
                        }
                    }
                    else if (paint_type == PaintType::SELECT_AND_PAINT && ImGui::IsMouseDown(0) && !brush_tiles.empty())
                    {
                        vec2 grid_pos = pos_to_grid(mouse_pos);
                        grid_pos.x = CLAMP(grid_pos.x, static_cast<int>(map->GetGridSize().x) - brush_size.x, 0);
                        grid_pos.y = CLAMP(grid_pos.y, static_cast<int>(map->GetGridSize().y) - brush_size.y, 0);

                        int brush_index = 0;
                        for (int y = 0; y < brush_size.y; ++y) {
                            for (int x = 0; x < brush_size.x; ++x) {
                                int target_x = grid_pos.x + x;
                                int target_y = grid_pos.y + y;
                                if (target_x < map->GetGridSize().x && target_y < map->GetGridSize().y) {
                                    map->SetTile({ target_x, target_y }, brush_tiles[brush_index++]);
                                }
                            }
                        }
                    }
                    else if (paint_type == PaintType::PAINT_BY_SELECTION)
                    {
                        if (ImGui::IsMouseClicked(0)) {
                            const bool ctrl_pressed = ImGui::GetIO().KeyCtrl;
                            ctrl_pressed_during_click = ctrl_pressed;
                            if (!ctrl_pressed) selected_tiles.clear();
                            is_selecting = true;
                            selection_start = pos_to_grid(mouse_pos);
                            selection_end = selection_start;
                        }

                        if (is_selecting) {
                            selection_end = pos_to_grid(mouse_pos);

                            const ImVec2 start = {
                                p.x - scroll_x + (float)std::min(selection_start.x, selection_end.x) * cell_size,
                                p.y - scroll_y + (float)std::min(selection_start.y, selection_end.y) * cell_size
                            };
                            const ImVec2 end = {
                                p.x - scroll_x + ((float)std::max(selection_start.x, selection_end.x) + 1) * cell_size,
                                p.y - scroll_y + ((float)std::max(selection_start.y, selection_end.y) + 1) * cell_size
                            };
                            draw_list->AddRectFilled(start, end, IM_COL32(100, 150, 255, 40));
                        }

                        if (ImGui::IsMouseReleased(0)) {
                            is_selecting = false;

                            const bool is_drag = (selection_start != selection_end);

                            if (is_drag)
                            {
                                const int x1 = CLAMP(static_cast<int>(std::min(selection_start.x, selection_end.x)), static_cast<int>(map->GetGridSize().x) - 1, 0);
                                const int x2 = CLAMP(static_cast<int>(std::max(selection_start.x, selection_end.x)), static_cast<int>(map->GetGridSize().x) - 1, 0);
                                const int y1 = CLAMP(static_cast<int>(std::min(selection_start.y, selection_end.y)), static_cast<int>(map->GetGridSize().y) - 1, 0);
                                const int y2 = CLAMP(static_cast<int>(std::max(selection_start.y, selection_end.y)), static_cast<int>(map->GetGridSize().y) - 1, 0);

                                const bool ctrl_now = ImGui::GetIO().KeyCtrl;

                                for (int y = y1; y <= y2; ++y) {
                                    for (int x = x1; x <= x2; ++x) {
                                        auto tile = std::make_pair(x, y);
                                        if (ctrl_now) {
                                            if (selected_tiles.count(tile)) selected_tiles.erase(tile);
                                            else selected_tiles.insert(tile);
                                        }
                                        else {
                                            selected_tiles.insert(tile);
                                        }
                                    }
                                }
                            }
                            else if (ctrl_pressed_during_click)
                            {
                                auto tile = std::make_pair(selection_start.x, selection_start.y);
                                if (selected_tiles.count(tile)) selected_tiles.erase(tile);
                                else selected_tiles.insert(tile);
                            }
                            else
                            {
                                auto tile = std::make_pair(selection_start.x, selection_start.y);
                                selected_tiles.insert(tile);
                            }
                        }
                    }

                    if (ImGui::IsMouseClicked(1)) {
                        vec2 grid_pos = pos_to_grid(mouse_pos);
                        grid_pos.x = CLAMP(grid_pos.x, static_cast<int>(map->GetGridSize().x) - 1, 0);
                        grid_pos.y = CLAMP(grid_pos.y, static_cast<int>(map->GetGridSize().y) - 1, 0);

                        if (paint_type == PaintType::SELECT_AND_PAINT) {
                            for (int y = 0; y < brush_size.y; ++y) {
                                for (int x = 0; x < brush_size.x; ++x) {
                                    int target_x = grid_pos.x + x;
                                    int target_y = grid_pos.y + y;
                                    if (target_x < map->GetGridSize().x && target_y < map->GetGridSize().y) {
                                        map->SetTile({ target_x, target_y }, -1);
                                    }
                                }
                            }
                        }
                        else if (paint_type == PaintType::BUCKET_PAINT)
                        {
                            BucketFill(grid_pos, -1);
                        }
                        else {
                            if (!selected_tiles.empty()) {
                                for (const auto& [x, y] : selected_tiles) {
                                    map->SetTile({ x, y }, -1);
                                }
                                selected_tiles.clear();
                            }
                            else {
                                map->SetTile(grid_pos, -1);
                                selected_tiles.erase({ grid_pos.x, grid_pos.y });
                            }
                        }
                    }
                }

                const ImVec2 visible_min = ImVec2(p.x - scroll_x, p.y - scroll_y);
                const ImVec2 visible_max = ImVec2(p.x + available_width - scroll_x, p.y + available_height - scroll_y);

                for (int y = 0; y < map->GetGridSize().y; ++y) {
                    for (int x = 0; x < map->GetGridSize().x; ++x) {
                        const ImVec2 rect_min(
                            p.x + x * cell_size - scroll_x,
                            p.y + y * cell_size - scroll_y
                        );
                        const ImVec2 rect_max(
                            rect_min.x + cell_size,
                            rect_min.y + cell_size
                        );
                        if (ImGui::IsRectVisible(rect_min, rect_max))
                        {
                            if (selected_tiles.count({ x, y })) {
                                draw_list->AddRectFilled(rect_min, rect_max, IM_COL32(100, 150, 255, 100));
                            }

                            draw_list->AddRect(rect_min, rect_max, IM_COL32(255, 255, 255, 50));

                            const int tile_id = map->GetTileValue({ x, y });
                            if (tile_id >= 0) {
                                if (show_numbers)
                                {
                                    const std::string label = std::to_string(tile_id);
                                    const ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
                                    draw_list->AddText(
                                        ImVec2(
                                            rect_min.x + (cell_size - text_size.x) * 0.5f,
                                            rect_min.y + (cell_size - text_size.y) * 0.5f
                                        ),
                                        IM_COL32(255, 255, 255, 255),
                                        label.c_str()
                                    );
                                }
                                else
                                {
                                    int tex_w, tex_h;
                                    map->GetTextureSize(tex_w, tex_h);

                                    ML_Rect uvs = Canvas::GetUVs(map->GetTileSection({ x, y }), tex_w, tex_h);

                                    draw_list->AddImage(
                                        (ImTextureID)map->GetTextureID(),
                                        rect_min,
                                        rect_max,
                                        { uvs.x, uvs.h },
                                        { uvs.w, uvs.y }
                                    );
                                }
                            }
                        }
                    }
                }
                ImGui::EndChild();

                if (!selected_tiles.empty() && paint_type == PaintType::PAINT_BY_SELECTION) {
                    ImGui::Text("Selected Tiles: %d", (int)selected_tiles.size());
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Deselect")) {
                        selected_tiles.clear();
                    }
                }
                if (paint_type == PaintType::SELECT_AND_PAINT && !brush_tiles.empty()) {
                    ImGui::Text("Brush Size: %dx%d", brush_size.x, brush_size.y);
                }
            }
        }
        ImGui::EndChild();

        ImGui::Columns(1);
    }
    ImGui::End();

    return ret;
}

void PanelTileMap::UpdateBrushTiles()
{
    brush_tiles.clear();
    int start_x = std::min(texture_selection_start.x, texture_selection_end.x);
    int end_x = std::max(texture_selection_start.x, texture_selection_end.x);
    int start_y = std::min(texture_selection_start.y, texture_selection_end.y);
    int end_y = std::max(texture_selection_start.y, texture_selection_end.y);

    int tiles_per_row = texture_size.x / tilemap->GetImageSectionSize().x;
    brush_size = { end_x - start_x + 1, end_y - start_y + 1 };

    for (int y = start_y; y <= end_y; ++y) {
        for (int x = start_x; x <= end_x; ++x) {
            brush_tiles.push_back(x + y * tiles_per_row);
        }
    }
}

void PanelTileMap::BucketFill(vec2f start_pos, int new_id) {
    if (!tilemap) return;

    int original_id = tilemap->GetTileValue(start_pos);
    if (original_id == new_id) return;

    std::queue<vec2f> nodes;
    std::set<std::pair<int, int>> visited;

    nodes.push(start_pos);
    visited.insert({ start_pos.x, start_pos.y });

    vec2f grid_size = tilemap->GetGridSize();

    while (!nodes.empty()) {
        vec2f current = nodes.front();
        nodes.pop();

        tilemap->SetTile(current, new_id);

        vec2f neighbors[4] = {
            {current.x + 1, current.y},
            {current.x - 1, current.y},
            {current.x, current.y + 1},
            {current.x, current.y - 1}
        };

        for (const auto& neighbor : neighbors) {
            if (neighbor.x >= 0 && neighbor.x < grid_size.x &&
                neighbor.y >= 0 && neighbor.y < grid_size.y)
            {
                auto key = std::make_pair(neighbor.x, neighbor.y);
                if (visited.count(key) == 0 &&
                    tilemap->GetTileValue(neighbor) == original_id)
                {
                    nodes.push(neighbor);
                    visited.insert(key);
                }
            }
        }
    }
}

void PanelTileMap::HandleShortCuts()
{
    if (ImGui::GetIO().WantTextInput) return;

    if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT &&
        engine->input_em->GetKey(SDL_SCANCODE_M) == KEY_DOWN)
    {
        paint_type = PaintType::PAINT_BY_SELECTION;
        selected_tiles.clear();
        brush_tiles.clear();
    }
    else if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT &&
        engine->input_em->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
    {
        paint_type = PaintType::SELECT_AND_PAINT;
        selected_tiles.clear();
        brush_tiles.clear();
    }
    else if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT &&
        engine->input_em->GetKey(SDL_SCANCODE_G) == KEY_DOWN)
    {
        paint_type = PaintType::BUCKET_PAINT;
        selected_tiles.clear();
        brush_tiles.clear();
    }
    else if (engine->input_em->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT &&
        engine->input_em->GetKey(SDL_SCANCODE_U) == KEY_DOWN)
    {
        selected_tiles.clear();
        brush_tiles.clear();
    }
}

void PanelTileMap::SetMap(TileMap* tilemap)
{
	this->tilemap = tilemap;
}
