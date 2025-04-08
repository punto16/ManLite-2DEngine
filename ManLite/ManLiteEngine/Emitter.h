#ifndef __EMITTER_H__
#define __EMITTER_H__
#pragma once

#include "vector"
#include "memory"
#include "string"
#include "Defs.h"
#include <future>
#include <atomic>

class GameObject;
class Particle;
class FontData;

enum class SpawnType
{
    CONSTANT_SPAWN                  = 0,
    BURST_SPAWN                     = 1,
    CONSTANT_BURST_SPAWN            = 2
};

enum class RenderType
{
    SQUARE                          = 0,
    CIRCLE                          = 1,
    IMAGE                           = 2,
    CHARACTER                       = 3
};

class UpdateOptionsEnabled
{
public:
    bool final_speed = false;
    bool final_direction = false;
    bool final_color = false;
    bool final_scale = false;
    bool wind_effect = false;
    bool final_position = false;
    bool final_angular_speed = false;
};

class EmitterTypeManager
{
public:
    EmitterTypeManager(){}
    EmitterTypeManager(EmitterTypeManager* ref)
    {
        spawn_type = ref->spawn_type;
        update_options_enabled = ref->update_options_enabled;
        render_type = ref->render_type;
        force_transform = ref->force_transform;
    }

    SpawnType spawn_type = SpawnType::CONSTANT_SPAWN;
    UpdateOptionsEnabled update_options_enabled;
    RenderType render_type = RenderType::SQUARE;

    //Go transform affects already spawned particles, not only the ones about to spawn
    bool force_transform = false;
};

class Emitter {
public:
    Emitter(std::weak_ptr<GameObject> container_go, std::string name = "Emitter", bool enable = true);
    Emitter(const Emitter& emitter_to_copy, std::shared_ptr<GameObject> container_go);
    ~Emitter();

    //can also be used for play
    bool Init();
    bool Update(float dt);

    void Draw();

    bool Pause();
    bool Unpause();

    void SpawnParticles();
    void SafeAddParticle();

    //getters // setters
    std::vector<std::shared_ptr<Particle>>& GetParticles() { return particles; }

    EmitterTypeManager* GetEmitterTypeManager() { return emitter_type_manager; }

    std::string GetCharacters() { return characters; }
    void SetCharacters(std::string c) { characters = c; }

    std::weak_ptr<GameObject> GetContainerGO() const { return container_go; }
    std::string GetName() const { return emitter_name; }
    uint32_t GetID() const { return emitter_id; }
    bool IsEnabled() const { return enabled; }
    int GetMaxParticles() const { return max_particles; }
    int GetParticlesPerSpawn() const { return particles_amount_per_spawn; }
    float GetSpawnRate() const { return spawn_rate; }
    float GetSpawnTimer() const { return spawn_timer; }
    uint GetTextureID() const { return texture_id; }
    std::string GetTexturePath() const { return texture_path; }
    int GetTextureWidth() const { return tex_w; }
    int GetTextureHeight() const { return tex_h; }
    std::string GetFontPath() const { return font_path; }
    FontData* GetFont() const { return font; }
    bool IsPixelArt() const { return pixel_art; }

    // Particle stats getters
    float GetParticleDurationMin() const { return particle_duration_min; }
    float GetParticleDurationMax() const { return particle_duration_max; }
    const ML_Color& GetInitColorMin() const { return init_color_min; }
    const ML_Color& GetInitColorMax() const { return init_color_max; }
    const vec2f& GetInitPositionMin() const { return init_position_min; }
    const vec2f& GetInitPositionMax() const { return init_position_max; }
    const vec2f& GetInitDirectionMin() const { return init_direction_min; }
    const vec2f& GetInitDirectionMax() const { return init_direction_max; }
    float GetInitSpeedMin() const { return init_speed_min; }
    float GetInitSpeedMax() const { return init_speed_max; }
    float GetInitAngleSpeedMin() const { return init_angle_speed_min; }
    float GetInitAngleSpeedMax() const { return init_angle_speed_max; }
    float GetInitAngleMin() const { return init_angle_min; }
    float GetInitAngleMax() const { return init_angle_max; }
    const vec2f& GetInitScaleMin() const { return init_scale_min; }
    const vec2f& GetInitScaleMax() const { return init_scale_max; }

    // Final stats getters
    const ML_Color& GetFinalColorMin() const { return final_color_min; }
    const ML_Color& GetFinalColorMax() const { return final_color_max; }
    const vec2f& GetFinalPositionMin() const { return final_position_min; }
    const vec2f& GetFinalPositionMax() const { return final_position_max; }
    const vec2f& GetFinalDirectionMin() const { return final_direction_min; }
    const vec2f& GetFinalDirectionMax() const { return final_direction_max; }
    float GetFinalSpeedMin() const { return final_speed_min; }
    float GetFinalSpeedMax() const { return final_speed_max; }
    float GetFinalAngleSpeedMin() const { return final_angle_speed_min; }
    float GetFinalAngleSpeedMax() const { return final_angle_speed_max; }
    const vec2f& GetFinalScaleMin() const { return final_scale_min; }
    const vec2f& GetFinalScaleMax() const { return final_scale_max; }
    const vec2f& GetWindEffectMin() const { return wind_effect_min; }
    const vec2f& GetWindEffectMax() const { return wind_effect_max; }

    // Setters
    void SetName(const std::string& name) { emitter_name = name; }
    void SetEnabled(bool enable) { enabled = enable; }
    void SetMaxParticles(int max) { max_particles = max; }
    void SetParticlesPerSpawn(int amount) { particles_amount_per_spawn = amount; }
    void SetSpawnRate(float rate) { spawn_rate = rate; }
    void SetEmitterTypeManager(EmitterTypeManager* manager) { emitter_type_manager = manager; }
    void SwapTexture(std::string new_path);
    void SwapFont(std::string new_font);
    void SetPixelArt(bool enabled) { pixel_art = enabled; }

    // Particle stats setters
    void SetParticleDurationMin(float duration) { particle_duration_min = duration; }
    void SetParticleDurationMax(float duration) { particle_duration_max = duration; }
    void SetInitColorMin(const ML_Color& color) { init_color_min = color; }
    void SetInitColorMax(const ML_Color& color) { init_color_max = color; }
    void SetInitPositionMin(const vec2f& pos) { init_position_min = pos; }
    void SetInitPositionMax(const vec2f& pos) { init_position_max = pos; }
    void SetInitDirectionMin(const vec2f& dir) { init_direction_min = dir; }
    void SetInitDirectionMax(const vec2f& dir) { init_direction_max = dir; }
    void SetInitSpeedMin(float speed) { init_speed_min = speed; }
    void SetInitSpeedMax(float speed) { init_speed_max = speed; }
    void SetInitAngleSpeedMin(float angle) { init_angle_speed_min = angle; }
    void SetInitAngleSpeedMax(float angle) { init_angle_speed_max = angle; }
    void SetInitAngleMin(float angle) { init_angle_min = angle; }
    void SetInitAngleMax(float angle) { init_angle_max = angle; }
    void SetInitScaleMin(const vec2f& scale) { init_scale_min = scale; }
    void SetInitScaleMax(const vec2f& scale) { init_scale_max = scale; }

    // Final stats setters
    void SetFinalColorMin(const ML_Color& color) { final_color_min = color; }
    void SetFinalColorMax(const ML_Color& color) { final_color_max = color; }
    void SetFinalPositionMin(const vec2f& pos) { final_position_min = pos; }
    void SetFinalPositionMax(const vec2f& pos) { final_position_max = pos; }
    void SetFinalDirectionMin(const vec2f& dir) { final_direction_min = dir; }
    void SetFinalDirectionMax(const vec2f& dir) { final_direction_max = dir; }
    void SetFinalSpeedMin(float speed) { final_speed_min = speed; }
    void SetFinalSpeedMax(float speed) { final_speed_max = speed; }
    void SetFinalAngleSpeedMin(float angle) { final_angle_speed_min = angle; }
    void SetFinalAngleSpeedMax(float angle) { final_angle_speed_max = angle; }
    void SetFinalScaleMin(const vec2f& scale) { final_scale_min = scale; }
    void SetFinalScaleMax(const vec2f& scale) { final_scale_max = scale; }
    void SetWindEffectMin(const vec2f& wind) { wind_effect_min = wind; }
    void SetWindEffectMax(const vec2f& wind) { wind_effect_max = wind; }

private:
    std::weak_ptr<GameObject> container_go;
    std::string emitter_name = "";
    uint32_t emitter_id = 0;
    bool enabled = true;

    int max_particles = 100;
    int particles_amount_per_spawn = 1;

    float spawn_rate = 0.1f;
    float spawn_timer = 0.0f;

    EmitterTypeManager* emitter_type_manager = nullptr;

    //render settings
    uint texture_id;
    std::string texture_path = "";
    int tex_w, tex_h;
    std::future<uint> textureFuture;
    std::atomic<bool> textureLoading{ false };
    std::string characters = "";
    std::string font_path = "";
    FontData* font;
    bool pixel_art = false;

    //stats
    //spawn
    float particle_duration_min = 1.0f;
    float particle_duration_max = 1.0f;
    //initial
    ML_Color init_color_min = { 100, 0, 0, 255 };
    ML_Color init_color_max = { 255, 0, 0, 255 };
    vec2f init_position_min = { 0, 0 };
    vec2f init_position_max = { 0, 0 };
    vec2f init_direction_min = { -2, 1 };
    vec2f init_direction_max = { 2, 3 };
    float init_speed_min = 1.0f;
    float init_speed_max = 1.0f;
    float init_angle_speed_min = 0.0f;
    float init_angle_speed_max = 0.0f;
    float init_angle_min = 0.0f;
    float init_angle_max = 0.0f;
    vec2f init_scale_min = { 0.1, 0.1 };
    vec2f init_scale_max = { 0.1, 0.1 };
    //final
    ML_Color final_color_min = { 255, 255, 255, 255 };
    ML_Color final_color_max = { 255, 255, 255, 255 };
    vec2f final_position_min = { 0, 0 };
    vec2f final_position_max = { 0, 0 };
    vec2f final_direction_min = { 0, 1 };
    vec2f final_direction_max = { 0, 1 };
    float final_speed_min = 1.0f;
    float final_speed_max = 1.0f;
    float final_angle_speed_min = 0.0f;
    float final_angle_speed_max = 0.0f;
    vec2f final_scale_min = { 0.2, 0.2 };
    vec2f final_scale_max = { 0.2, 0.2 };
    vec2f wind_effect_min = { 0, 0 };
    vec2f wind_effect_max = { 0, 0 };
    //
    std::vector<std::shared_ptr<Particle>> particles;
};

#endif // !__EMITTER_H__