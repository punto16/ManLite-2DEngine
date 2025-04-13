#ifndef __TILEMAP_H__
#define __TILEMAP_H__
#pragma once

#include "Component.h"
#include "Defs.h"

class TileMap : public Component {
public:
    TileMap(std::weak_ptr<GameObject> container_go, std::string name = "TileMap", bool enable = true);
    TileMap(const TileMap& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~TileMap();

    void Draw() override;

    //serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    //getters // setters
    int GetTile(vec2 tile) const;
    vec2 GetTile(int tile) const;
    void SetTile(vec2 tile, int value);
    void ResizeGrid(vec2 grid_size, int default_value = 0);
    ML_Rect GetTileSection(vec2 tile) const;

private:
    
    inline bool IsValidPosition(vec2 pos) const;

    std::vector<int> grid_data;

    vec2 grid_size = { 1, 1 };
    vec2f tile_size = { 1.0f, 1.0f };

    vec2f image_section_size = { 1.0f, 1.0f };

};

#endif // !__TILEMAP_H__