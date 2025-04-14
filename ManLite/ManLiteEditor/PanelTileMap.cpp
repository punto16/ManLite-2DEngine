#include "PanelTileMap.h"

#include "GUI.h"
#include "ResourceManager.h"
#include "TileMap.h"
#include "Log.h"
#include "Canvas.h"

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
            this->enabled = false;
            return true;
        }

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
        ImGui::EndChild();

        ImGui::NextColumn();
        ImGui::BeginChild("PropertiesPanel", ImVec2(0, 0), true);
        {
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
                ImGui::Text("Selected Tile: %d", selected_tile_id);
                if (selected_tile_id >= 0) {
                    int tex_w, tex_h;
                    map->GetTextureSize(tex_w, tex_h);
                    int tiles_per_row = tex_w / map->GetImageSectionSize().x;
                    ImGui::Text("Tile Position: (%d, %d)",
                        selected_tile_id % tiles_per_row,
                        selected_tile_id / tiles_per_row);
                }

                ImGui::Checkbox("Show Numbers", &show_numbers);
            }

            if (ImGui::CollapsingHeader("Grid Preview", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const float available_width = ImGui::GetContentRegionAvail().x;
                const float available_height = ImGui::GetContentRegionAvail().y - 80;

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

                    if (ImGui::IsMouseClicked(1)) {
                        if (!selected_tiles.empty()) {
                            for (const auto& [x, y] : selected_tiles) {
                                map->SetTile({ x, y }, -1);
                            }
                            selected_tiles.clear();
                        }
                        else {
                            vec2 grid_pos = pos_to_grid(mouse_pos);
                            grid_pos.x = CLAMP(grid_pos.x, static_cast<int>(map->GetGridSize().x) - 1, 0);
                            grid_pos.y = CLAMP(grid_pos.y, static_cast<int>(map->GetGridSize().y) - 1, 0);
                            map->SetTile(grid_pos, -1);
                            selected_tiles.erase({ grid_pos.x, grid_pos.y });
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

                                    ML_Rect uvs = Canvas::GetUVs(tilemap->GetTileSection({ x, y }), tex_w, tex_h);

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

                if (!selected_tiles.empty()) {
                    ImGui::Text("Selected Tiles: %d", (int)selected_tiles.size());
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Clear Selection")) {
                        selected_tiles.clear();
                    }
                }
                if (selected_tile_id >= 0) {
                    ImGui::Text("Last Selected Tile ID: %d", selected_tile_id);
                }
            }
        }
        ImGui::EndChild();

        ImGui::Columns(1);
    }
    ImGui::End();

    return ret;
}

void PanelTileMap::SetMap(TileMap* tilemap)
{
	this->tilemap = tilemap;
}
