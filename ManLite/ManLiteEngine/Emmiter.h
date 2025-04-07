#ifndef __EMMITER_H__
#define __EMMITER_H__
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

class EmmiterTypeManager
{
public:
    EmmiterTypeManager(){}
    EmmiterTypeManager(EmmiterTypeManager* ref)
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

class Emmiter {
public:
    Emmiter(std::weak_ptr<GameObject> container_go, std::string name = "Emmiter", bool enable = true);
    Emmiter(const Emmiter& emmiter_to_copy, std::shared_ptr<GameObject> container_go);
    ~Emmiter();

    //can also be used for play
    bool Init();
    bool Update(float dt);

    void Draw();

    bool Pause();
    bool Unpause();

    void SpawnParticles();
    void SafeAddParticle();

    //getters // setters
    std::vector<std::shared_ptr<Particle>>& GetParticles() { return particles; };

    EmmiterTypeManager* GetEmmiterTypeManager() { return emmiter_type_manager; }


    std::string GetCharacters() { return characters; }

private:
    std::weak_ptr<GameObject> container_go;
    std::string emmiter_name = "";
    uint32_t emmiter_id = 0;
    bool enabled = true;

    int max_particles = 100;
    int particles_amount_per_spawn = 1;

    float spawn_rate = 1.0f;
    float spawn_timer = 0.0f;

    EmmiterTypeManager* emmiter_type_manager = nullptr;

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
    float particle_duration_min;
    float particle_duration_max;
    //initial
    ML_Color init_color_min;
    ML_Color init_color_max;
    vec2f init_position_min;
    vec2f init_position_max;
    vec2f init_direction_min;
    vec2f init_direction_max;
    float init_speed_min;
    float init_speed_max;
    float init_angle_speed_min;
    float init_angle_speed_max;
    float init_angle_min;
    float init_angle_max;
    vec2f init_scale_min;
    vec2f init_scale_max;
    //final
    ML_Color final_color_min;
    ML_Color final_color_max;
    vec2f final_position_min;
    vec2f final_position_max;
    vec2f final_direction_min;
    vec2f final_direction_max;
    float final_speed_min;
    float final_speed_max;
    float final_angle_speed_min;
    float final_angle_speed_max;
    vec2f final_scale_min;
    vec2f final_scale_max;
    vec2f wind_effect_min;
    vec2f wind_effect_max;
    //
    std::vector<std::shared_ptr<Particle>> particles;
};

#endif // !__EMMITER_H__