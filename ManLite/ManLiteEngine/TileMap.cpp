#include "TileMap.h"

#include "GameObject.h"
#include "Transform.h"
#include "Sprite2D.h"
#include "EngineCore.h"
#include "RendererEM.h"
#include "Canvas.h"

TileMap::TileMap(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
	Component(container_go, ComponentType::TileMap, name, enable),
	tile_size({ 1, 1 }),
	image_section_size({ 32, 32 }),
	texturePath(""),
	textureID(0),
	tex_width(0),
	tex_height(0),
	pixel_art(true)
{
	int x, y;
	ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", x, y);
	ResizeGrid({ 1, 1 }, -1);
}

TileMap::TileMap(const TileMap& component_to_copy, std::shared_ptr<GameObject> container_go) :
	Component(component_to_copy, container_go),
	tile_size(component_to_copy.tile_size),
	image_section_size(component_to_copy.image_section_size),
	textureID(0),
	tex_width(0),
	tex_height(0),
	pixel_art(component_to_copy.pixel_art)
{
	int x, y;
	ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", x, y);
	if (!texturePath.empty()) ResourceManager::GetInstance().ReleaseTexture(texturePath);
	texturePath = component_to_copy.texturePath;
	textureID = ResourceManager::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
	ResizeGrid(component_to_copy.GetGridSize(), -1);
	grid_data = std::vector<int>(component_to_copy.grid_data);

}

TileMap::~TileMap()
{
	grid_data.clear();
	ResourceManager::GetInstance().ReleaseTexture("Config\\placeholder.png");
	if (!texturePath.empty()) ResourceManager::GetInstance().ReleaseTexture(texturePath);
}

void TileMap::Draw()
{
	if (textureLoading && textureFuture.valid()) {
		if (textureFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			textureID = textureFuture.get();
			textureLoading = false;
		}
	}

	mat3f mat;
	if (auto t = container_go.lock()->GetComponent<Transform>())
	{
		mat = t->GetWorldMatrix();
	}

	for (size_t i = 0; i < grid_data.size(); ++i)
	{
		const vec2 grid_pos = GetTile(static_cast<int>(i));

		if (GetTileValue(grid_pos) == -1) continue;

		const vec2f local_pos = {
			(grid_pos.x * tile_size.x) + (tile_size.x / 2.0f),
			(-grid_pos.y * tile_size.y) - (tile_size.y / 2.0f)
		};

		mat3f final_mat = mat3f::CreateTransformMatrix(
			local_pos,
			0,
			tile_size);

		final_mat = mat * final_mat;

		ML_Rect uvs = Canvas::GetUVs(GetTileSection(grid_pos), tex_width, tex_height);

		engine->renderer_em->SubmitSprite(
			textureID != 0 ? textureID : ResourceManager::GetInstance().GetTexture("Config\\placeholder.png"),
			final_mat,
			uvs.x, uvs.y, uvs.w, uvs.h,
			pixel_art
		);
	}
}

nlohmann::json TileMap::SaveComponent()
{
	nlohmann::json componentJSON;
	//component generic
	componentJSON["ContainerGOID"] = this->container_go.lock()->GetID();
	componentJSON["ComponentID"] = component_id;
	componentJSON["ComponentName"] = name;
	componentJSON["ComponentType"] = (int)type;
	componentJSON["Enabled"] = enabled;

	//component specific
	componentJSON["GridSize"] = { grid_size.x, grid_size.y };
	componentJSON["TileSize"] = { tile_size.x, tile_size.y };
	componentJSON["ImageSectionSize"] = { image_section_size.x, image_section_size.y };
	componentJSON["TexturePath"] = texturePath;
	componentJSON["PixelArt"] = pixel_art;

	for (size_t i = 0; i < grid_data.size(); i++)
	{
		componentJSON["GridData"][i] = grid_data[i];
	}
	return componentJSON;
}

void TileMap::LoadComponent(const nlohmann::json& componentJSON)
{
	if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
	if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
	if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
	if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];

	if (componentJSON.contains("TexturePath"))
	{
		texturePath = componentJSON["TexturePath"];
		textureLoading = true;
		textureFuture = ResourceManager::GetInstance().LoadTextureAsync(texturePath, tex_width, tex_height);
	}
	if (componentJSON.contains("GridSize"))
	{
		ResizeGrid({ componentJSON["GridSize"][0] , componentJSON["GridSize"][1] }, -1);
	}
	if (componentJSON.contains("TileSize"))
	{
		tile_size = { componentJSON["TileSize"][0] , componentJSON["TileSize"][1] };
	}
	if (componentJSON.contains("ImageSectionSize"))
	{
		image_section_size = { componentJSON["ImageSectionSize"][0] , componentJSON["ImageSectionSize"][1] };
	}
	if (componentJSON.contains("PixelArt")) pixel_art = componentJSON["PixelArt"];
	if (componentJSON.contains("GridData"))
	{
		for (size_t i = 0; i < grid_data.size(); i++)
		{
			grid_data[i] = componentJSON["GridData"][i];
		}
	}
}

int TileMap::GetTileValue(vec2 tile) const
{
	if (!IsValidPosition(tile)) return -1;
	return grid_data[tile.y * grid_size.x + tile.x];
}

vec2 TileMap::GetTile(int tile_index) const
{
	if (tile_index < 0 || tile_index >= grid_size.x * grid_size.y)
		return { -1, -1 };

	int x = grid_size.x;

	return {
		static_cast<int>(tile_index % x),
		static_cast<int>(tile_index / grid_size.x)
	};
}

void TileMap::SetTile(vec2 tile, int value)
{
	if (!IsValidPosition(tile)) return;
	grid_data[tile.y * grid_size.x + tile.x] = value;
}

void TileMap::ResizeGrid(vec2 new_grid_size, int default_value)
{
	std::vector<int> old_grid = std::move(grid_data);
	vec2 old_size = grid_size;

	grid_size = new_grid_size;
	grid_data.resize(grid_size.x * grid_size.y, default_value);

	const int copy_width = std::min(old_size.x, grid_size.x);
	const int copy_height = std::min(old_size.y, grid_size.y);

	for (int y = 0; y < copy_height; ++y) {
		for (int x = 0; x < copy_width; ++x) {
			const int old_index = y * old_size.x + x;
			const int new_index = y * grid_size.x + x;

			if (old_index < static_cast<int>(old_grid.size()) &&
				new_index < static_cast<int>(grid_data.size())) {
				grid_data[new_index] = old_grid[old_index];
			}
		}
	}
}

ML_Rect TileMap::GetTileSection(vec2 tile) const
{
	if (!IsValidPosition(tile)) return { 0, 0, 0, 0 };

	int tile_id = GetTileValue(tile);
	if (tile_id < 0) return { 0, 0, 0, 0 };

	int tiles_per_row = static_cast<int>(tex_width / image_section_size.x);
	if (tiles_per_row <= 0) tiles_per_row = 1;

	int column = tile_id % tiles_per_row;
	int row = tile_id / tiles_per_row;

	int x = static_cast<int>(column * image_section_size.x);
	int y = static_cast<int>(row * image_section_size.y);

	if (x + image_section_size.x > tex_width ||
		y + image_section_size.y > tex_height) {
		return { 0, 0, 0, 0 };
	}

	return {
		x,
		y,
		static_cast<int>(image_section_size.x),
		static_cast<int>(image_section_size.y)
	};
}

void TileMap::SwapTexture(std::string new_path)
{
	if (!texturePath.empty()) ResourceManager::GetInstance().ReleaseTexture(texturePath);
	texturePath = new_path;
	textureID = ResourceManager::GetInstance().LoadTexture(texturePath, tex_width, tex_height);
}

bool TileMap::IsValidPosition(vec2 pos) const {
	return pos.x >= 0 && pos.x < grid_size.x &&
		pos.y >= 0 && pos.y < grid_size.y;
}
