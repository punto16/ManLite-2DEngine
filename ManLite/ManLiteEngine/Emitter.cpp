#include "Emitter.h"

#include "GameObject.h"
#include "Transform.h"
#include "ParticleSystem.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "EngineCore.h"
#include "RendererEM.h"
#include "SceneManagerEM.h"

Emitter::Emitter(std::weak_ptr<GameObject> container_go, std::string name, bool enable) :
	container_go(container_go),
	emitter_name(name),
	enabled(enable),
	emitter_type_manager(new EmitterTypeManager()),
	emitter_id(GameObject::GenerateGameObjectID()),
	max_particles(10000),
	particles_amount_per_spawn(1),
	spawn_rate(0.1f),
	spawn_timer(0.0f),
	texture_id(0),
	texture_path(""),
	characters(""),
	font_path(""),
	font(nullptr),
	pixel_art(false),
	particle_duration_min(1.0f),
	particle_duration_max(1.0f),
	init_color_min({ 100, 0, 0, 255 }),
	init_color_max({ 255, 0, 0, 255 }),
	init_position_min({0, 0}),
	init_position_max({ 0, 0 }),
	init_direction_min({ -2, 1 }),
	init_direction_max({ 2, 3 }),
	init_speed_min(1.0f),
	init_speed_max(1.0f),
	init_angle_speed_min(0.0f),
	init_angle_speed_max(0.0f),
	init_angle_min(0.0f),
	init_angle_max(0.0f),
	init_scale_min({0.1, 0.1 }),
	init_scale_max({ 0.1, 0.1 }),
	final_color_min({ 255, 255, 255, 255 }),
	final_color_max({ 255, 255, 255, 255 }),
	final_position_min({ 0, 0 }),
	final_position_max({ 0, 0 }),
	final_direction_min({ 0, 1 }),
	final_direction_max({ 0, 1 }),
	final_speed_min(1.0f),
	final_speed_max(1.0f),
	final_angle_speed_min(0.0f),
	final_angle_speed_max(0.0f),
	final_scale_min({ 0.2, 0.2 }),
	final_scale_max({ 0.2, 0.2 }),
	wind_effect_min({ 0, 0 }),
	wind_effect_max({ 0, 0 })
{
	ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_w, tex_h);//load placeholder

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
	emitter_id(GameObject::GenerateGameObjectID()),
	max_particles(emitter_to_copy.max_particles),
	particles_amount_per_spawn(emitter_to_copy.particles_amount_per_spawn),
	spawn_rate(emitter_to_copy.spawn_rate),
	spawn_timer(emitter_to_copy.spawn_timer),
	texture_id(0),
	texture_path(emitter_to_copy.texture_path),
	characters(emitter_to_copy.characters),
	font_path(emitter_to_copy.font_path),
	font(nullptr),
	pixel_art(emitter_to_copy.pixel_art),
	particle_duration_min(emitter_to_copy.particle_duration_min),
	particle_duration_max(emitter_to_copy.particle_duration_max),
	init_color_min(emitter_to_copy.init_color_min),
	init_color_max(emitter_to_copy.init_color_max),
	init_position_min(emitter_to_copy.init_position_min),
	init_position_max(emitter_to_copy.init_position_max),
	init_direction_min(emitter_to_copy.init_direction_min),
	init_direction_max(emitter_to_copy.init_direction_max),
	init_speed_min(emitter_to_copy.init_speed_min),
	init_speed_max(emitter_to_copy.init_speed_max),
	init_angle_speed_min(emitter_to_copy.init_angle_speed_min),
	init_angle_speed_max(emitter_to_copy.init_angle_speed_max),
	init_angle_min(emitter_to_copy.init_angle_min),
	init_angle_max(emitter_to_copy.init_angle_max),
	init_scale_min(emitter_to_copy.init_scale_min),
	init_scale_max(emitter_to_copy.init_scale_max),
	final_color_min(emitter_to_copy.final_color_min),
	final_color_max(emitter_to_copy.final_color_max),
	final_position_min(emitter_to_copy.final_position_min),
	final_position_max(emitter_to_copy.final_position_max),
	final_direction_min(emitter_to_copy.final_direction_min),
	final_direction_max(emitter_to_copy.final_direction_max),
	final_speed_min(emitter_to_copy.final_speed_min),
	final_speed_max(emitter_to_copy.final_speed_max),
	final_angle_speed_min(emitter_to_copy.final_angle_speed_min),
	final_angle_speed_max(emitter_to_copy.final_angle_speed_max),
	final_scale_min(emitter_to_copy.final_scale_min),
	final_scale_max(emitter_to_copy.final_scale_max),
	wind_effect_min(emitter_to_copy.wind_effect_min),
	wind_effect_max(emitter_to_copy.wind_effect_max)
{

	particle_pool.reserve(max_particles);
	for (size_t i = 0; i < max_particles; ++i) {
		particle_pool.emplace_back(this);
		available_indices.push(i);
	}

	ResourceManager::GetInstance().LoadTexture("Config\\placeholder.png", tex_w, tex_h);//load placeholder
	texture_id = ResourceManager::GetInstance().LoadTexture(texture_path, tex_w, tex_h);
	if (!font_path.empty()) font = ResourceManager::GetInstance().LoadFont(font_path,512);
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

	std::vector<size_t> active_copy;
	{
		std::lock_guard<std::mutex> lock(pool_mutex);
		if (active_indices.empty()) return true;
		active_copy.assign(active_indices.begin(), active_indices.end());
	}

	const size_t num_active = active_copy.size();
	std::atomic<size_t> current_idx(0);
	std::vector<std::future<void>> futures;
	const size_t num_threads = thread_pool.GetThreadCount(); // Asegúrate de implementar esto

	// Dividir el trabajo en tareas
	for (size_t t = 0; t < num_threads; ++t) {
		futures.emplace_back(thread_pool.enqueue([&, dt]() {
			std::vector<size_t> local_to_remove;
			while (true) {
				size_t idx = current_idx.fetch_add(1, std::memory_order_relaxed);
				if (idx >= num_active) break;
				size_t particle_idx = active_copy[idx];
				particle_pool[particle_idx].Update(dt);
				if (particle_pool[particle_idx].finished) {
					local_to_remove.push_back(particle_idx);
				}
			}
			if (!local_to_remove.empty()) {
				std::lock_guard<std::mutex> lock(removal_mutex);
				to_remove_global.insert(to_remove_global.end(), local_to_remove.begin(), local_to_remove.end());
			}
			}));
	}

	// Esperar a que todas las tareas terminen
	for (auto& future : futures) future.wait();

	// Procesar partículas terminadas
	{
		std::lock_guard<std::mutex> lock(pool_mutex);
		for (auto idx : to_remove_global) {
			active_indices.erase(std::remove(active_indices.begin(), active_indices.end(), idx), active_indices.end());
			available_indices.push(idx);
		}
		to_remove_global.clear();
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

			engine->renderer_em->SubmitDebugCollider(
				particle_t,
				particle.color,
				false,
				engine->scene_manager_em->GetCurrentScene().GetGOOrderInLayer(container_go.lock()),
				0.0f,
				0.0f,
				true
			);
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

			engine->renderer_em->SubmitDebugCollider(
				particle_t,
				particle.color,
				true,
				engine->scene_manager_em->GetCurrentScene().GetGOOrderInLayer(container_go.lock()),
				0.0f,
				particle.scale.x / 2,
				true);
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
				pixel_art,
				engine->scene_manager_em->GetCurrentScene().GetGOOrderInLayer(container_go.lock())
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
	if (!container_go.lock()->GetComponent<ParticleSystem>()->IsStop())
		for (size_t i = 0; i < particles_amount_per_spawn; i++)
			SafeAddParticle();

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
	active_indices.push_back(idx);
}

nlohmann::json Emitter::SaveComponent()
{
	nlohmann::json emitterJSON;

	emitterJSON["EmitterName"] = emitter_name;
	emitterJSON["Enabled"] = enabled;

	emitterJSON["MaxParticles"] = max_particles;
	emitterJSON["ParticlesPerSpawn"] = particles_amount_per_spawn;
	emitterJSON["SpawnRate"] = spawn_rate;

	emitterJSON["EmitterTypeManager"][0] = emitter_type_manager->spawn_type;
	emitterJSON["EmitterTypeManager"][1][0] = emitter_type_manager->update_options_enabled.final_speed;
	emitterJSON["EmitterTypeManager"][1][1] = emitter_type_manager->update_options_enabled.final_direction;
	emitterJSON["EmitterTypeManager"][1][2] = emitter_type_manager->update_options_enabled.final_color;
	emitterJSON["EmitterTypeManager"][1][3] = emitter_type_manager->update_options_enabled.final_scale;
	emitterJSON["EmitterTypeManager"][1][4] = emitter_type_manager->update_options_enabled.wind_effect;
	emitterJSON["EmitterTypeManager"][1][5] = emitter_type_manager->update_options_enabled.final_position;
	emitterJSON["EmitterTypeManager"][1][6] = emitter_type_manager->update_options_enabled.final_angular_speed;
	emitterJSON["EmitterTypeManager"][2] = emitter_type_manager->render_type;

	emitterJSON["TexturePath"] = texture_path;
	emitterJSON["Characters"] = characters;
	emitterJSON["FontPath"] = font_path;
	emitterJSON["PixelArt"] = pixel_art;

	emitterJSON["ParticleDurationMin"] = particle_duration_min;
	emitterJSON["ParticleDurationMax"] = particle_duration_max;
	emitterJSON["InitColorMin"] = { init_color_min.r, init_color_min.g, init_color_min.b, init_color_min.a };
	emitterJSON["InitColorMax"] = { init_color_max.r, init_color_max.g, init_color_max.b, init_color_max.a };
	emitterJSON["InitPositionMin"] = { init_position_min.x, init_position_min.y };
	emitterJSON["InitPositionMax"] = { init_position_max.x, init_position_max.y };
	emitterJSON["InitDirectionMin"] = { init_direction_min.x, init_direction_min.y };
	emitterJSON["InitDirectionMax"] = { init_direction_max.x, init_direction_max.y };
	emitterJSON["InitSpeedMin"] = init_speed_min;
	emitterJSON["InitSpeedMax"] = init_speed_max;
	emitterJSON["InitAngleSpeedMin"] = init_angle_speed_min;
	emitterJSON["InitAngleSpeedMax"] = init_angle_speed_max;
	emitterJSON["InitAnglMin"] = init_angle_min;
	emitterJSON["InitAnglMax"] = init_angle_max;
	emitterJSON["InitScaleMin"] = { init_scale_min.x, init_scale_min.y };
	emitterJSON["InitScaleMax"] = { init_scale_max.x, init_scale_max.y };

	emitterJSON["FinalColorMin"] = { final_color_min.r, final_color_min.g, final_color_min.b, final_color_min.a };
	emitterJSON["FinalColorMax"] = { final_color_max.r, final_color_max.g, final_color_max.b, final_color_max.a };
	emitterJSON["FinalPositionMin"] = { final_position_min.x, final_position_min.y };
	emitterJSON["FinalPositionMax"] = { final_position_max.x, final_position_max.y };
	emitterJSON["FinalDirectionMin"] = { final_direction_min.x, final_direction_min.y };
	emitterJSON["FinalDirectionMax"] = { final_direction_max.x, final_direction_max.y };
	emitterJSON["FinalSpeedMin"] = final_speed_min;
	emitterJSON["FinalSpeedMax"] = final_speed_max;
	emitterJSON["FinalAngleSpeedMin"] = final_angle_speed_min;
	emitterJSON["FinalAngleSpeedMax"] = final_angle_speed_max;
	emitterJSON["FinalScaleMin"] = { final_scale_min.x, final_scale_min.y };
	emitterJSON["FinalScaleMax"] = { final_scale_max.x, final_scale_max.y };
	emitterJSON["WindEffectMin"] = { wind_effect_min.x, wind_effect_min.y };
	emitterJSON["WindEffectMax"] = { wind_effect_max.x, wind_effect_max.y };

	return emitterJSON;
}

void Emitter::LoadComponent(const nlohmann::json& emitterJSON)
{
	if (emitterJSON.contains("EmitterName")) emitter_name = emitterJSON["EmitterName"];
	if (emitterJSON.contains("Enabled")) enabled = emitterJSON["Enabled"];

	if (emitterJSON.contains("MaxParticles")) max_particles = emitterJSON["MaxParticles"];
	if (emitterJSON.contains("ParticlesPerSpawn")) particles_amount_per_spawn = emitterJSON["ParticlesPerSpawn"];
	if (emitterJSON.contains("SpawnRate")) spawn_rate = emitterJSON["SpawnRate"];

	if (emitterJSON.contains("EmitterTypeManager"))
	{
		emitter_type_manager->spawn_type = emitterJSON["EmitterTypeManager"][0];
		emitter_type_manager->update_options_enabled.final_speed = emitterJSON["EmitterTypeManager"][1][0];
		emitter_type_manager->update_options_enabled.final_direction = emitterJSON["EmitterTypeManager"][1][1];
		emitter_type_manager->update_options_enabled.final_color = emitterJSON["EmitterTypeManager"][1][2];
		emitter_type_manager->update_options_enabled.final_scale = emitterJSON["EmitterTypeManager"][1][3];
		emitter_type_manager->update_options_enabled.wind_effect = emitterJSON["EmitterTypeManager"][1][4];
		emitter_type_manager->update_options_enabled.final_position = emitterJSON["EmitterTypeManager"][1][5];
		emitter_type_manager->update_options_enabled.final_angular_speed = emitterJSON["EmitterTypeManager"][1][6];
		emitter_type_manager->render_type = emitterJSON["EmitterTypeManager"][2];
	}

	if (emitterJSON.contains("TexturePath"))
	{
		texture_path = emitterJSON["TexturePath"];
		textureLoading = true;
		textureFuture = ResourceManager::GetInstance().LoadTextureAsync(texture_path, tex_w, tex_h);
	}
	if (emitterJSON.contains("Characters")) characters = emitterJSON["Characters"];
	if (emitterJSON.contains("FontPath")) font_path = emitterJSON["FontPath"];
	if (emitterJSON.contains("PixelArt")) pixel_art = emitterJSON["PixelArt"];

	if (emitterJSON.contains("ParticleDurationMin")) particle_duration_min = emitterJSON["ParticleDurationMin"];
	if (emitterJSON.contains("ParticleDurationMax")) particle_duration_max = emitterJSON["ParticleDurationMax"];
	if (emitterJSON.contains("InitColorMin"))
	{
		init_color_min.r = emitterJSON["InitColorMin"][0];
		init_color_min.g = emitterJSON["InitColorMin"][1];
		init_color_min.b = emitterJSON["InitColorMin"][2];
		init_color_min.a = emitterJSON["InitColorMin"][3];
	}
	if (emitterJSON.contains("InitColorMax"))
	{
		init_color_max.r = emitterJSON["InitColorMax"][0];
		init_color_max.g = emitterJSON["InitColorMax"][1];
		init_color_max.b = emitterJSON["InitColorMax"][2];
		init_color_max.a = emitterJSON["InitColorMax"][3];
	}
	if (emitterJSON.contains("InitPositionMin"))
	{
		init_position_min.x = emitterJSON["InitPositionMin"][0];
		init_position_min.y = emitterJSON["InitPositionMin"][1];
	}
	if (emitterJSON.contains("InitPositionMax"))
	{
		init_position_max.x = emitterJSON["InitPositionMax"][0];
		init_position_max.y = emitterJSON["InitPositionMax"][1];
	}
	if (emitterJSON.contains("InitDirectionMin"))
	{
		init_direction_min.x = emitterJSON["InitDirectionMin"][0];
		init_direction_min.y = emitterJSON["InitDirectionMin"][1];
	}
	if (emitterJSON.contains("InitDirectionMax"))
	{
		init_direction_max.x = emitterJSON["InitDirectionMax"][0];
		init_direction_max.y = emitterJSON["InitDirectionMax"][1];
	}
	if (emitterJSON.contains("InitSpeedMin")) init_speed_min = emitterJSON["InitSpeedMin"];
	if (emitterJSON.contains("InitSpeedMax")) init_speed_max = emitterJSON["InitSpeedMax"];
	if (emitterJSON.contains("InitAngleSpeedMin")) init_angle_speed_min = emitterJSON["InitAngleSpeedMin"];
	if (emitterJSON.contains("InitAngleSpeedMax")) init_angle_speed_max = emitterJSON["InitAngleSpeedMax"];
	if (emitterJSON.contains("InitAngleMin")) init_angle_min = emitterJSON["InitSpeedMin"];
	if (emitterJSON.contains("InitAngleMax")) init_angle_max = emitterJSON["InitSpeedMax"];

	if (emitterJSON.contains("InitScaleMin"))
	{
		init_scale_min.x = emitterJSON["InitScaleMin"][0];
		init_scale_min.y = emitterJSON["InitScaleMin"][1];
	}
	if (emitterJSON.contains("InitScaleMax"))
	{
		init_scale_max.x = emitterJSON["InitScaleMax"][0];
		init_scale_max.y = emitterJSON["InitScaleMax"][1];
	}
	//
	if (emitterJSON.contains("FinalColorMin"))
	{
		final_color_min.r = emitterJSON["FinalColorMin"][0];
		final_color_min.g = emitterJSON["FinalColorMin"][1];
		final_color_min.b = emitterJSON["FinalColorMin"][2];
		final_color_min.a = emitterJSON["FinalColorMin"][3];
	}
	if (emitterJSON.contains("FinalColorMax"))
	{
		final_color_max.r = emitterJSON["FinalColorMax"][0];
		final_color_max.g = emitterJSON["FinalColorMax"][1];
		final_color_max.b = emitterJSON["FinalColorMax"][2];
		final_color_max.a = emitterJSON["FinalColorMax"][3];
	}
	if (emitterJSON.contains("FinalPositionMin"))
	{
		final_position_min.x = emitterJSON["FinalPositionMin"][0];
		final_position_min.y = emitterJSON["FinalPositionMin"][1];
	}
	if (emitterJSON.contains("FinalPositionMax"))
	{
		final_position_max.x = emitterJSON["FinalPositionMax"][0];
		final_position_max.y = emitterJSON["FinalPositionMax"][1];
	}
	if (emitterJSON.contains("FinalDirectionMin"))
	{
		final_direction_min.x = emitterJSON["FinalDirectionMin"][0];
		final_direction_min.y = emitterJSON["FinalDirectionMin"][1];
	}
	if (emitterJSON.contains("FinalDirectionMax"))
	{
		final_direction_max.x = emitterJSON["FinalDirectionMax"][0];
		final_direction_max.y = emitterJSON["FinalDirectionMax"][1];
	}
	if (emitterJSON.contains("FinalSpeedMin")) final_speed_min = emitterJSON["FinalSpeedMin"];
	if (emitterJSON.contains("FinalSpeedMax")) final_speed_max = emitterJSON["FinalSpeedMax"];
	if (emitterJSON.contains("FinalAngleSpeedMin")) final_angle_speed_min = emitterJSON["FinalAngleSpeedMin"];
	if (emitterJSON.contains("FinalAngleSpeedMax")) final_angle_speed_max = emitterJSON["FinalAngleSpeedMax"];
	if (emitterJSON.contains("FinalScaleMin"))
	{
		final_scale_min.x = emitterJSON["FinalScaleMin"][0];
		final_scale_min.y = emitterJSON["FinalScaleMin"][1];
	}
	if (emitterJSON.contains("FinalScaleMax"))
	{
		final_scale_max.x = emitterJSON["FinalScaleMax"][0];
		final_scale_max.y = emitterJSON["FinalScaleMax"][1];
	}
	if (emitterJSON.contains("WindEffectMin"))
	{
		wind_effect_min.x = emitterJSON["WindEffectMin"][0];
		wind_effect_min.y = emitterJSON["WindEffectMin"][1];
	}
	if (emitterJSON.contains("WindEffectMax"))
	{
		wind_effect_max.x = emitterJSON["WindEffectMax"][0];
		wind_effect_max.y = emitterJSON["WindEffectMax"][1];
	}
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
