#ifndef __PARTICLE_H__
#define __PARTICLE_H__
#pragma once

#include "Defs.h"
#include <memory>

class Emitter;

class Particle : public std::enable_shared_from_this<Particle> {
public:
    Particle(Emitter* container_emitter);
    ~Particle();

    void Reset();
    void Restart();

    bool Update(float dt);

    //getters // setters

public:
    //how much time particle will be alive
    float duration;
    //current duration since spawn to now
    float life_time;

    //init stuff
    //color
    ML_Color init_color;

    //transform
    vec2f init_position;
    vec2f init_direction;
    float init_speed;
    float init_angle;
    float init_angle_speed;
    vec2f init_scale;

    //current stuff
    //color
    ML_Color color;
    
    //transform
    vec2f position;
    vec2f direction;
    float speed;
    float angle;
    vec2f scale;


    //final stuff
    //color
    ML_Color final_color;

    //transform
    vec2f final_position;
    vec2f final_direction;
    float final_speed;
    float final_angle_speed;
    vec2f final_scale;
    vec2f wind_effect;

    //particle management
    bool finished = false;
    bool loop = true;

    //
    std::string char_to_print = "";

    Emitter* container_emitter = nullptr;
};

#endif // !__PARTICLE_H__