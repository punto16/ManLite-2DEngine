#include "PanelTileMap.h"

#include "GUI.h"
#include "ResourceManager.h"
#include "TileMap.h"
#include "Log.h"

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

        // Panel dividido
        ImGui::Columns(2, "TileMapEditor", true);

        // --- Columna izquierda: Tileset ---
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

                // Dibujar grid
                for (int x = 0; x <= columns; ++x) {
                    const float x_pos = canvas_pos.x + x * tile_w;
                    draw_list->AddLine(ImVec2(x_pos, canvas_pos.y), ImVec2(x_pos, canvas_pos.y + canvas_size.y), IM_COL32(255, 255, 255, 100));
                }
                for (int y = 0; y <= rows; ++y) {
                    const float y_pos = canvas_pos.y + y * tile_h;
                    draw_list->AddLine(ImVec2(canvas_pos.x, y_pos), ImVec2(canvas_pos.x + canvas_size.x, y_pos), IM_COL32(255, 255, 255, 100));
                }

                // Selección de tile
                const ImVec2 mouse_pos = ImGui::GetMousePos();
                if (ImGui::IsMouseClicked(0))
                {
                    const float rel_x = (mouse_pos.x - canvas_pos.x) / scale;
                    const float rel_y = (mouse_pos.y - canvas_pos.y) / scale;
                    selected_tile_id = static_cast<int>(rel_x / map->GetImageSectionSize().x) +
                        static_cast<int>(rel_y / map->GetImageSectionSize().y) * columns;

                    // Aplicar a selección múltiple
                    if (!selected_tiles.empty()) {
                        for (const auto& [x, y] : selected_tiles) {
                            map->SetTile({ x, y }, selected_tile_id);
                        }
                    }
                }
            }
        }
        ImGui::EndChild();

        // --- Columna derecha: Propiedades ---
        ImGui::NextColumn();
        ImGui::BeginChild("PropertiesPanel", ImVec2(0, 0), true);
        {
            // Configuración
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
            }

            // Vista previa del grid
            if (ImGui::CollapsingHeader("Grid Preview", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const float available_width = ImGui::GetContentRegionAvail().x;
                const float available_height = ImGui::GetContentRegionAvail().y - 50; // Espacio para texto
                const float preview_width = available_width;
                const float preview_height = available_height;

                ImGui::BeginChild("GridPreview", ImVec2(preview_width, preview_height), true);
                {
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    const ImVec2 p = ImGui::GetCursorScreenPos();
                    const ImVec2 mouse_pos = ImGui::GetMousePos();

                    // Tamaño adaptable
                    const float cell_size = std::min(
                        preview_width * 0.8f / map->GetGridSize().x,
                        preview_height * 0.8f / map->GetGridSize().y
                    );
                    const float total_width = cell_size * map->GetGridSize().x;
                    const float total_height = cell_size * map->GetGridSize().y;
                    const float offset_x = 0.8f * (preview_width - total_width) * 0.5f;
                    const float offset_y = 0.8f * (preview_height - total_height) * 0.5f;

                    auto pos_to_grid = [&](const ImVec2& pos) -> vec2 {
                        const float grid_x = (pos.x - p.x - offset_x) / cell_size;
                        const float grid_y = (pos.y - p.y - offset_y) / cell_size;

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

                            // Dibujar selección temporal
                            const ImVec2 start = {
                                p.x + offset_x + (float)std::min(selection_start.x, selection_end.x) * cell_size,
                                p.y + offset_y + (float)std::min(selection_start.y, selection_end.y) * cell_size
                            };
                            const ImVec2 end = {
                                p.x + offset_x + ((float)std::max(selection_start.x, selection_end.x) + 1) * cell_size,
                                p.y + offset_y + ((float)std::max(selection_start.y, selection_end.y) + 1) * cell_size
                            };
                            draw_list->AddRectFilled(start, end, IM_COL32(100, 150, 255, 40));
                        }

                        if (ImGui::IsMouseReleased(0)) {
                            is_selecting = false;

                            const bool is_drag = (selection_start != selection_end);

                            if (is_drag)
                            {
                                // Calcular límites con precisión
                                const int x1 = CLAMP(static_cast<int>(std::min(selection_start.x, selection_end.x)), static_cast<int>(map->GetGridSize().x) - 1, 0);
                                const int x2 = CLAMP(static_cast<int>(std::max(selection_start.x, selection_end.x)), static_cast<int>(map->GetGridSize().x) - 1, 0);
                                const int y1 = CLAMP(static_cast<int>(std::min(selection_start.y, selection_end.y)), static_cast<int>(map->GetGridSize().y) - 1, 0);
                                const int y2 = CLAMP(static_cast<int>(std::max(selection_start.y, selection_end.y)), static_cast<int>(map->GetGridSize().y) - 1, 0);

                                // Estado actual de Ctrl al soltar
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

                        // Click derecho para borrar
                        if (ImGui::IsMouseClicked(1)) {
                            if (!selected_tiles.empty()) {
                                // Borrar todos los tiles seleccionados
                                for (const auto& [x, y] : selected_tiles) {
                                    map->SetTile({ x, y }, -1);
                                }
                                selected_tiles.clear();
                            }
                            else {
                                // Borrar solo el tile bajo el cursor
                                vec2 grid_pos = pos_to_grid(mouse_pos);
                                grid_pos.x = CLAMP(grid_pos.x, static_cast<int>(map->GetGridSize().x) - 1, 0);
                                grid_pos.y = CLAMP(grid_pos.y, static_cast<int>(map->GetGridSize().y) - 1, 0);
                                map->SetTile(grid_pos, -1);
                                selected_tiles.erase({ grid_pos.x, grid_pos.y });
                            }
                        }
                    }

                    // Dibujar celdas
                    for (int y = 0; y < map->GetGridSize().y; ++y) {
                        for (int x = 0; x < map->GetGridSize().x; ++x) {
                            const ImVec2 rect_min(
                                p.x + offset_x + x * cell_size,
                                p.y + offset_y + y * cell_size
                            );
                            const ImVec2 rect_max(
                                rect_min.x + cell_size,
                                rect_min.y + cell_size
                            );

                            // Resaltar selección
                            if (selected_tiles.count({ x, y })) {
                                draw_list->AddRectFilled(rect_min, rect_max, IM_COL32(100, 150, 255, 100));
                            }

                            // Dibujar celda
                            draw_list->AddRect(rect_min, rect_max, IM_COL32(255, 255, 255, 50));

                            // Mostrar ID del tile
                            const int tile_id = map->GetTileValue({ x, y });
                            if (tile_id >= 0) {
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
                        }
                    }
                }
                ImGui::EndChild();

                // Texto DESPUÉS del grid
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
