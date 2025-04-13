#include "TileMap.h"

#include "GameObject.h"
#include "Transform.h"
#include "Sprite2D.h"
#include "EngineCore.h"
#include "RendererEM.h"
#include "Canvas.h"

TileMap::TileMap(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
	Component(container_go, ComponentType::TileMap, name, enable)
{
	int x, y;
	ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", x, y);
}

TileMap::TileMap(const TileMap& component_to_copy, std::shared_ptr<GameObject> container_go) :
	Component(component_to_copy, container_go)
{
	int x, y;
	ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", x, y);
}

TileMap::~TileMap()
{
	grid_data.clear();
	ResourceManager::GetInstance().ReleaseTexture("Config\\placeholder.png");
}

void TileMap::Draw()
{
	auto sprite = container_go.lock()->GetComponent<Sprite2D>();
	uint texture_id;
	if (!sprite) texture_id = ResourceManager::GetInstance().GetTexture("Config\\placeholder.png");
	else if (sprite->GetTextureID() == 0) texture_id = ResourceManager::GetInstance().GetTexture("Config\\placeholder.png");
	else texture_id = sprite->GetTextureID();

	int tex_w, tex_h = 0;
	sprite->GetTextureSize(tex_w, tex_h);

	mat3f mat;
	if (auto t = container_go.lock()->GetComponent<Transform>())
	{
		mat = t->GetWorldMatrix();
	}

	for (size_t i = 0; i < grid_data.size(); ++i)
	{
		const vec2 grid_pos = GetTile(static_cast<int>(i));

		const vec2f local_pos = {
			(grid_pos.x * tile_size.x) + (tile_size.x / 2.0f),
			(grid_pos.y * tile_size.y) + (tile_size.y / 2.0f)
		};

		mat3f final_mat = mat3f::CreateTransformMatrix(
			local_pos,
			0,
			tile_size);

		final_mat = mat * final_mat;

		ML_Rect uvs = Canvas::GetUVs(GetTileSection(grid_pos), tex_w, tex_h);

		engine->renderer_em->SubmitSprite(
			texture_id,
			final_mat,
			uvs.x, uvs.y, uvs.w, uvs.h,
			sprite ? sprite->IsPixelArt() : false
		);
	}
}

int TileMap::GetTile(vec2 tile) const
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
		static_cast<float>(tile_index % x),
		static_cast<float>(tile_index / grid_size.x)
	};
}

void TileMap::SetTile(vec2 tile, int value)
{
	if (!IsValidPosition(tile)) return;
	grid_data[tile.y * grid_size.x + tile.x] = value;
}

void TileMap::ResizeGrid(vec2 grid_size, int default_value)
{
	this->grid_size.x = grid_size.x;
	this->grid_size.y = grid_size.y;
	grid_data.resize(grid_size.x * grid_size.y, default_value);
}

ML_Rect TileMap::GetTileSection(vec2 tile) const
{
	if (!IsValidPosition(tile)) return { 0, 0, 0, 0 };
	auto sprite = container_go.lock()->GetComponent<Sprite2D>();
	if (!sprite) return { 0, 0, 0, 0 };
	int tex_w, tex_h = 0;
	sprite->GetTextureSize(tex_w, tex_h);

	int tile_id = GetTile(tile);
	if (tile_id < 0) return { 0, 0, 0, 0 };

	int tiles_per_row = static_cast<int>(tex_w / image_section_size.x);
	if (tiles_per_row <= 0) tiles_per_row = 1;

	int column = tile_id % tiles_per_row;
	int row = tile_id / tiles_per_row;

	int x = static_cast<int>(column * image_section_size.x);
	int y = static_cast<int>(row * image_section_size.y);

	if (x + image_section_size.x > tex_w ||
		y + image_section_size.y > tex_h) {
		return { 0, 0, 0, 0 };
	}

	return {
		x,
		y,
		static_cast<int>(image_section_size.x),
		static_cast<int>(image_section_size.y)
	};
}

bool TileMap::IsValidPosition(vec2 pos) const {
	return pos.x >= 0 && pos.x < grid_size.x &&
		pos.y >= 0 && pos.y < grid_size.y;
}
