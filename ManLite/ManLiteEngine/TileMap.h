#ifndef __TILEMAP_H__
#define __TILEMAP_H__
#pragma once

#include "Component.h"
#include "Defs.h"

#include <future>
#include <atomic>

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
    int GetTileValue(vec2 tile) const;
    vec2 GetTile(int tile) const;
    void SetTile(vec2 tile, int value);
    void ResizeGrid(vec2 new_grid_size, int default_value = -1);
    vec2 GetGridSize() const { return grid_size; }
    ML_Rect GetTileSection(vec2 tile) const;
    //size in pixels
    void SetTileSize(vec2 px_size) 
    {
        tile_size.x = PIXEL_TO_METERS(px_size.x);
        tile_size.y = PIXEL_TO_METERS(px_size.y);
    }
    //size in pixels
    vec2 GetTileSize() const { return { METERS_TO_PIXELS(tile_size.x), METERS_TO_PIXELS(tile_size.y) }; }
    void SetImageSectionSize(vec2 px_size) { image_section_size = px_size; }
    vec2 GetImageSectionSize() const { return image_section_size; }

    uint GetTextureID() const { return textureID; }
    const std::string& GetTexturePath() const { return texturePath; }
    void GetTextureSize(int& tex_width, int& tex_height)
    {
        tex_width = this->tex_width;
        tex_height = this->tex_height;
    }
    void SetPixelArtRender(bool pixel_art) { this->pixel_art = pixel_art; }
    bool IsPixelArt() { return pixel_art; }

    void SwapTexture(std::string new_path);

private:
    
    inline bool IsValidPosition(vec2 pos) const;

    //grid
    std::vector<int> grid_data;

    //grid data
    vec2 grid_size = { 1, 1 };
    vec2f tile_size = { 1.0, 1.0 };
    vec2f image_section_size = { 32, 32 };

    //text data
    std::string texturePath = "";
    uint textureID = 0;
    int tex_width, tex_height = 0;
    bool pixel_art = true;

    std::future<uint> textureFuture;
    std::atomic<bool> textureLoading{ false };
};

#endif // !__TILEMAP_H__