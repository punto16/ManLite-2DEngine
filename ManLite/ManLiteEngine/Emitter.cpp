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

	particle_pool.reserve(max_particles);
	for (size_t i = 0; i < max_particles; ++i) {
		particle_pool.emplace_back(this);
		available_indices.push(i);
	}
}

Emitter::Emitter(const Emitter& emitter_to_copy, std::shared_ptr<GameObject> container_go) :
	container_go(container_go),
	emitter_name(emitter_to_copy.emitter_name),
	enabled(emitter_to_copy.enabled),
	emitter_type_manager(new EmitterTypeManager(emitter_to_copy.emitter_type_manager)),
	emitter_id(GameObject::GenerateGameObjectID())
{
	max_particles = emitter_to_copy.max_particles;

	particle_pool.reserve(max_particles);
	for (size_t i = 0; i < max_particles; ++i) {
		particle_pool.emplace_back(this);
		available_indices.push(i);
	}

	particles_amount_per_spawn = emitter_to_copy.particles_amount_per_spawn;
	spawn_rate = emitter_to_copy.spawn_rate;
	texture_id = emitter_to_copy.texture_id;
	tex_w = emitter_to_copy.tex_w;
	tex_h = emitter_to_copy.tex_h;

	ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_w, tex_h);//load placeholder
}

Emitter::~Emitter()
{
	ResourceManager::GetInstance().ReleaseTexture("Config\\placeholder.png");
	ResourceManager::GetInstance().ReleaseTexture(texture_path);
	if (font) ResourceManager::GetInstance().ReleaseFont(font_path);
	delete emitter_type_manager;
	std::lock_guard<std::mutex> lock(pool_mutex);
}

bool Emitter::Init()
{
	bool ret = true;

	SpawnParticles();

	return ret;
}

bool Emitter::Update(float dt)
{
	if (!enabled) return true;

	if (emitter_type_manager->spawn_type != SpawnType::BURST_SPAWN)
	{
		spawn_timer += dt;
		if (spawn_timer >= spawn_rate)
		{
			SpawnParticles();
			spawn_timer = 0.0f;
		}
	}

	{
		std::lock_guard<std::mutex> lock(pool_mutex);

		if (active_indices.empty()) return true;

		std::vector<size_t> active_copy(active_indices.begin(), active_indices.end());
		const size_t num_active = active_copy.size();

		const unsigned num_threads = std::min<unsigned>(
			std::thread::hardware_concurrency(),
			num_active
		);

		std::vector<std::thread> threads;
		threads.reserve(num_threads);

		const size_t chunk_size = num_active / num_threads;
		size_t current_idx = 0;

		for (unsigned t = 0; t < num_threads; ++t)
		{
			const size_t start = current_idx;
			const size_t end = (t == num_threads - 1)
				? num_active
				: start + chunk_size;
			current_idx = end;

			threads.emplace_back([this, dt, &active_copy, start, end]() {
				for (size_t i = start; i < end; ++i) {
					particle_pool[active_copy[i]].Update(dt);
				}
				});
		}

		for (auto& thread : threads) {
			if (thread.joinable()) thread.join();
		}
	}

	{
		std::lock_guard<std::mutex> lock(pool_mutex);

		std::vector<size_t> to_remove;
		for (const auto& idx : active_indices) {
			if (particle_pool[idx].finished) {
				available_indices.push(idx);
				to_remove.push_back(idx);
			}
		}

		for (const auto& idx : to_remove) {
			active_indices.erase(idx);
		}
	}

	return true;
}

void Emitter::Draw()
{
	std::lock_guard<std::mutex> lock(pool_mutex);

	switch (emitter_type_manager->render_type)
	{
	case RenderType::SQUARE:
	{
		for (const auto idx : active_indices) {
			const auto& particle = particle_pool[idx];
			if (particle.finished) continue;

			mat3f mat;
			
			if (auto t = container_go.lock()->GetComponent<Transform>())
			{
				mat = t->GetWorldMatrix();
			}

			mat3f particle_t = mat3f::CreateTransformMatrix(
				particle.position,
				DEGTORAD * particle.angle,
				{ particle.scale.x, particle.scale.y });

			particle_t = mat * particle_t;

			engine->renderer_em->SubmitDebugCollider(particle_t, particle.color, false, 0.0f, true);
		}
		break;
	}
	case RenderType::CIRCLE:
	{
		for (const auto idx : active_indices) {
			const auto& particle = particle_pool[idx];
			if (particle.finished) continue;

			mat3f mat;

			if (auto t = container_go.lock()->GetComponent<Transform>())
			{
				mat = t->GetWorldMatrix();
			}

			mat3f particle_t = mat3f::CreateTransformMatrix(
				particle.position,
				DEGTORAD * particle.angle,
				{ 1, 1 });
			particle_t = mat * particle_t;

			engine->renderer_em->SubmitDebugCollider(particle_t, particle.color, true, particle.scale.x / 2, true);
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

		for (const auto idx : active_indices) {
			const auto& particle = particle_pool[idx];
			if (particle.finished) continue;

			mat3f mat;

			if (auto t = container_go.lock()->GetComponent<Transform>())
			{
				mat = t->GetWorldMatrix();
			}

			mat3f particle_t = mat3f::CreateTransformMatrix(
				particle.position,
				DEGTORAD * particle.angle,
				{ particle.scale.x * tex_w / tex_h, particle.scale.y });
			particle_t = mat * particle_t;

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
		for (const auto idx : active_indices) {
			const auto& particle = particle_pool[idx];
			if (particle.finished) continue;

			mat3f mat;

			if (auto t = container_go.lock()->GetComponent<Transform>())
			{
				mat = t->GetWorldMatrix();
			}

			mat3f particle_t = mat3f::CreateTransformMatrix(
				particle.position,
				DEGTORAD * particle.angle,
				{ particle.scale.x * tex_w / tex_h * 0.005, particle.scale.y * 0.005 });
			particle_t = mat * particle_t;

			engine->renderer_em->SubmitText(particle.char_to_print, font, particle_t, particle.color, TextAlignment::TEXT_ALIGN_CENTER);
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
	std::lock_guard<std::mutex> lock(pool_mutex);

	if (available_indices.empty()) return;

	const size_t idx = available_indices.front();
	available_indices.pop();

	Particle& p = particle_pool[idx];
	p.Reset();

	//init stats
	p.duration = RandomRange(particle_duration_min, particle_duration_max);
	p.init_color = RandomRange(init_color_min, init_color_max);
	p.init_position = RandomRange(init_position_min, init_position_max);
	p.init_direction = RandomRange(init_direction_min, init_direction_max);
	p.init_speed = RandomRange(init_speed_min, init_speed_max);
	p.init_angle_speed = RandomRange(init_angle_speed_min, init_angle_speed_max);
	p.init_angle = RandomRange(init_angle_min, init_angle_max);
	p.init_scale = RandomRange(init_scale_min, init_scale_max);
	//final stats
	p.final_color = RandomRange(final_color_min, final_color_max);
	p.final_position = RandomRange(final_position_min, final_position_max);
	p.final_direction = RandomRange(final_direction_min, final_direction_max);
	p.final_speed = RandomRange(final_speed_min, final_speed_max);
	p.final_angle_speed = RandomRange(final_angle_speed_min, final_angle_speed_max);
	p.final_scale = RandomRange(final_scale_min, final_scale_max);
	p.wind_effect = RandomRange(wind_effect_min, wind_effect_max);
	p.Restart();
	active_indices.insert(idx);
}

int Emitter::GetActiveParticlesCount()
{
	std::lock_guard<std::mutex> lock(pool_mutex);
	return static_cast<int>(active_indices.size());
}

void Emitter::SetMaxParticles(int max)
{
	std::lock_guard<std::mutex> lock(pool_mutex);

	if (max < 0 || max == max_particles) return;

	const int old_max = max_particles;
	max_particles = max;

	if (max < old_max)
	{
		for (auto it = active_indices.begin(); it != active_indices.end();)
		{
			if (*it >= static_cast<size_t>(max))
			{
				particle_pool[*it].finished = true;
				it = active_indices.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	particle_pool.resize(max);

	std::queue<size_t> new_available;
	for (size_t i = 0; i < max_particles; ++i)
	{
		if (std::find(active_indices.begin(), active_indices.end(), i) == active_indices.end())
		{
			new_available.push(i);
		}
	}
	available_indices = std::move(new_available);
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
