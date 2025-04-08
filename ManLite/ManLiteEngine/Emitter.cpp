#include "Emitter.h"

#include "GameObject.h"
#include "Transform.h"
#include "ParticleSystem.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "EngineCore.h"
#include "RendererEM.h"

Emitter::Emitter(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
	container_go(container_go),
	emitter_name(name),
	enabled(enable),
	emitter_type_manager(new EmitterTypeManager()),
	emitter_id(GameObject::GenerateGameObjectID())
{
	ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_w, tex_h);//load placeholder
	font = nullptr;
}

Emitter::Emitter(const Emitter& emitter_to_copy, std::shared_ptr<GameObject> container_go) :
	container_go(container_go),
	emitter_name(emitter_to_copy.emitter_name),
	enabled(emitter_to_copy.enabled),
	emitter_type_manager(new EmitterTypeManager(emitter_to_copy.emitter_type_manager)),
	emitter_id(GameObject::GenerateGameObjectID())
{
	ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_w, tex_h);//load placeholder
}

Emitter::~Emitter()
{
	ResourceManager::GetInstance().ReleaseTexture("Config\\placeholder.png");
	ResourceManager::GetInstance().ReleaseTexture(texture_path);
	if (font) ResourceManager::GetInstance().ReleaseFont(font_path);
	particles.clear();
	delete emitter_type_manager;
}

bool Emitter::Init()
{
	bool ret = true;

	SpawnParticles();

	return ret;
}

bool Emitter::Update(float dt)
{
	bool ret = true;

	if (emitter_type_manager->spawn_type != SpawnType::BURST_SPAWN)
	{
		spawn_timer += dt;
		if (spawn_timer >= spawn_rate) SpawnParticles();
	}

	auto particles_list = std::vector<std::shared_ptr<Particle>>(particles);
	for (const auto& particle : particles_list)
	{
		particle->Update(dt);

		auto t = container_go.lock()->GetComponent<Transform>();
		if (emitter_type_manager->force_transform && t)
		{
			particle->position += t->GetWorldPosition();
			particle->angle = t->GetWorldAngle();
			particle->init_position += t->GetWorldPosition();
			particle->final_position += t->GetWorldPosition();
		}
	}

	return ret;
}

void Emitter::Draw()
{
	switch (emitter_type_manager->render_type)
	{
	case RenderType::SQUARE:
	{
		for (const auto& particle : particles)
		{
			mat3f particle_t = mat3f::CreateTransformMatrix(
				particle->position,
				DEGTORAD * particle->angle,
				{ particle->scale.x, particle->scale.y });

			engine->renderer_em->SubmitDebugCollider(particle_t, particle->color, false, 0.0f, true);
		}
		break;
	}
	case RenderType::CIRCLE:
	{
		for (const auto& particle : particles)
		{
			mat3f particle_t = mat3f::CreateTransformMatrix(
				particle->position,
				DEGTORAD * particle->angle,
				{ particle->scale.x, particle->scale.y });

			engine->renderer_em->SubmitDebugCollider(particle_t, particle->color, true, particle->scale.x / 2, true);
		}
		break;
	}
	case RenderType::IMAGE:
	{
		if (!enabled) return;

		if (textureLoading && textureFuture.valid()) {
			if (textureFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
				texture_id = textureFuture.get();
				textureLoading = false;
			}
		}

		for (const auto& particle : particles)
		{
			mat3f particle_t = mat3f::CreateTransformMatrix(
				particle->position,
				DEGTORAD * particle->angle,
				{ particle->scale.x * tex_w / tex_h, particle->scale.y });

			engine->renderer_em->SubmitSprite(
				texture_id != 0 ? texture_id : ResourceManager::GetInstance().GetTexture("Config\\placeholder.png"),
				particle_t,
				0, 1, 1, 0,
				pixel_art
			);

		}

		break;
	}
	case RenderType::CHARACTER:
		for (const auto& particle : particles)
		{
			mat3f particle_t = mat3f::CreateTransformMatrix(
				particle->position,
				DEGTORAD * particle->angle,
				{ particle->scale.x * tex_w / tex_h * 0.005, particle->scale.y * 0.005 });

			engine->renderer_em->SubmitText(particle->char_to_print, font, particle_t, particle->color, TextAlignment::TEXT_ALIGN_CENTER);
		}
		break;
	default:
		break;
	}
}

bool Emitter::Pause()
{
	bool ret = true;

	return ret;
}

bool Emitter::Unpause()
{
	bool ret = true;

	return ret;
}

void Emitter::SpawnParticles()
{
	for (size_t i = 0; i < particles_amount_per_spawn; i++)
	{
		SafeAddParticle();
	}

	spawn_timer = 0.0f;
}

void Emitter::SafeAddParticle()
{
	auto t = container_go.lock()->GetComponent<Transform>();
	if (!t) return;

	if (particles.size() >= max_particles)
	{
		particles[0]->Restart();
	}
	else
	{
		std::shared_ptr<Particle> p = std::make_shared<Particle>(this);
		//init stats
		p->duration = RandomRange(particle_duration_min, particle_duration_max);
		p->init_color = RandomRange(init_color_min, init_color_max);
		p->init_position = RandomRange(init_position_min + t->GetWorldPosition(), init_position_max + t->GetWorldPosition());
		p->init_direction = RandomRange(init_direction_min, init_direction_max);
		p->init_speed = RandomRange(init_speed_min, init_speed_max);
		p->init_angle_speed = RandomRange(init_angle_speed_min, init_angle_speed_max);
		p->init_angle = RandomRange(init_angle_min + t->GetWorldAngle(), init_angle_max + t->GetWorldAngle());
		p->init_scale = RandomRange(init_scale_min, init_scale_max);
		//final stats
		p->final_color = RandomRange(final_color_min, final_color_max);
		p->final_position = RandomRange(final_position_min + t->GetWorldPosition(), final_position_max + t->GetWorldPosition());
		p->final_direction = RandomRange(final_direction_min, final_direction_max);
		p->final_speed = RandomRange(final_speed_min, final_speed_max);
		p->final_angle_speed = RandomRange(final_angle_speed_min, final_angle_speed_max);
		p->final_scale = RandomRange(final_scale_min, final_scale_max);
		p->wind_effect = RandomRange(wind_effect_min, wind_effect_max);
		p->Restart();
		particles.push_back(p);
	}
}

void Emitter::SwapFont(std::string new_font)
{
	if (font) ResourceManager::GetInstance().ReleaseFont(font_path);
	font_path = new_font;
	font = ResourceManager::GetInstance().LoadFont(font_path, 512);
}

void Emitter::SwapTexture(std::string new_path)
{
	ResourceManager::GetInstance().ReleaseTexture(texture_path);
	texture_path = new_path;
	texture_id = ResourceManager::GetInstance().LoadTexture(texture_path, tex_w, tex_h);
}
