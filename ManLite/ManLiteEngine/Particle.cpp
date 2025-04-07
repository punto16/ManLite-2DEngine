#include "Particle.h"

#include "Emmiter.h"

#include "Defs.h"

Particle::Particle(Emmiter* container_emmiter) :
	container_emmiter(container_emmiter)
{
	Reset();
}

Particle::~Particle()
{
}

void Particle::Reset()
{
	duration = 10.0f;
	life_time = 0.0f;
	//init
	init_color = { 255, 255, 255, 255 };
	init_position = { 0, 0 };
	init_direction = { 0, 1 };
	init_speed = 1.0f;
	init_angle = 0.0f;
	init_scale = { 1, 1 };

	//current
	color = { 255, 255, 255, 255 };
	position = { 0, 0 };
	direction = { 0, 1 };
	speed = 1.0f;
	angle = 0.0f;
	scale = { 1, 1 };

	//final
	final_color = { 255, 255, 255, 255 };
	final_position = { 0.0f, 0.0f };
	final_speed = 1.0f;
	final_angle_speed = 0.0f;
	final_scale = { 1.0f, 1.0f };
	wind_effect = { 0.0f, 0.0f };

	char_to_print = RandomCharacter(container_emmiter->GetCharacters());
}

void Particle::Restart()
{
	auto self = shared_from_this();
	auto& particles = container_emmiter->GetParticles();
	particles.erase(
		std::remove_if(particles.begin(), particles.end(),
			[self](const std::shared_ptr<Particle>& c) {
				return c == self;
			}),
		particles.end()
	);
	particles.push_back(self);

	life_time = 0.0f;
	finished = false;

	color = init_color;
	position = init_position;
	direction = init_direction;
	speed = init_speed;
	angle = init_angle;
	scale = init_scale;
}

bool Particle::Update(float dt)
{
	bool ret = true;

	if (finished && loop) Restart();
	life_time += dt;
	if (life_time >= duration) finished = true;

	float t = life_time / duration;
	t = std::clamp(t, 0.0f, 1.0f);
	UpdateOptionsEnabled* update_type = &container_emmiter->GetEmmiterTypeManager()->update_options_enabled;

	if (update_type->final_color)
	{
		color.r = static_cast<int>(std::lerp(init_color.r, final_color.r, t));
		color.g = static_cast<int>(std::lerp(init_color.g, final_color.g, t));
		color.b = static_cast<int>(std::lerp(init_color.b, final_color.b, t));
		color.a = static_cast<int>(std::lerp(init_color.a, final_color.a, t));
	}

	if (update_type->final_direction)
	{
		direction.x = std::lerp(init_direction.x, final_direction.x, t);
		direction.y = std::lerp(init_direction.y, final_direction.y, t);
	}

	if (update_type->final_speed)
		speed = std::lerp(init_speed, final_speed, t);

	if (update_type->final_angular_speed)
		angle += std::lerp(init_angle_speed, final_angle_speed, t) * dt;

	if (update_type->final_scale)
	{
		scale.x = std::lerp(init_scale.x, final_scale.x, t);
		scale.y = std::lerp(init_scale.y, final_scale.y, t);
	}

	//regular movement
	position = {
		position.x + direction.x * speed * dt,
		position.y + direction.y * speed * dt
	};

	//wind effect
	if (update_type->wind_effect)
	{
		position = {
		position.x + speed * RandomRange(-wind_effect.x / 2, wind_effect.x / 2),
		position.y + speed * RandomRange(-wind_effect.y / 2, wind_effect.y / 2)
		};
	}

	//reach final position
	if (update_type->final_position)
	{
		position = {
		std::lerp(position.x, final_position.x, t),
		std::lerp(position.y, final_position.y, t)
		};
	}

	return ret;
}
