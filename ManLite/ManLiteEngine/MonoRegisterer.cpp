#include "MonoRegisterer.h"

#include "GameObject.h"
#include "ScriptingEM.h"

#include "EngineCore.h"
#include "InputEM.h"
#include "SceneManagerEM.h"

#include "Transform.h"
#include "Camera.h"
#include "Sprite2D.h"
#include "Animator.h"
#include "Collider2D.h"
#include "Canvas.h"
#include "UIElement.h"
#include "ImageUI.h"
#include "ButtonImageUI.h"
#include "CheckBoxUI.h"
#include "SliderUI.h"
#include "TextUI.h"
#include "AudioSource.h"
#include "ParticleSystem.h"
#include "TileMap.h"

#include "Defs.h"
#include "SDL2/SDL.h"

static GameObject* GetGOPtr()
{
	return ScriptingEM::GetGOPtr();
}

static MonoString* GetGOName(GameObject* go)
{
	if (!go) return MonoRegisterer::ToMonoString("");
	return MonoRegisterer::ToMonoString(go->GetName());
}

static void SetGOName(GameObject* go, MonoString* name)
{
	if (!go) return;
	go->SetTag(MonoRegisterer::ToCppString(name));
}

static MonoString* GetGOTag(GameObject* go)
{
	if (!go) return MonoRegisterer::ToMonoString("");
	return MonoRegisterer::ToMonoString(go->GetTag());
}

static void SetGOTag(GameObject* go, MonoString* tag)
{
	if (!go) return;
	go->SetTag(MonoRegisterer::ToCppString(tag));
}

static unsigned int GetGOID(GameObject* go)
{
	if (!go) return 0;
	return go->GetID();
}

static GameObject* GetGOParent(GameObject* go)
{
	if (!go) return nullptr;
	if (!go->GetParentGO().lock()) return nullptr;
	return go->GetParentGO().lock().get();
}

static bool GetGOEnabled(GameObject* go)
{
	if (!go) return false;
	return go->IsEnabled();
}

static void SetGOEnabled(GameObject* go, bool enable)
{
	if (!go) return;
	go->SetEnabled(enable);
}

static void SwitchGOEnabled(GameObject* go)
{
	if (!go) return;
	go->SwitchEnabled();
}

static bool GetGOVisible(GameObject* go)
{
	if (!go) return false;
	return go->IsVisible();
}

static void SetGOVisible(GameObject* go, bool visible)
{
	if (!go) return;
	go->SetVisible(visible);
}

static void SwitchGOVisible(GameObject* go)
{
	if (!go) return;
	go->SwitchVisible();
}

static void ReparentGO(GameObject* go, GameObject* new_parent)
{
	if (!go) return;
	go->Reparent(new_parent->GetSharedPtr());
}


static bool GetKeyboardKeyDown(int id)
{
	return engine->input_em->GetKey(id) == KEY_DOWN;
}
static bool GetKeyboardKeyRepeat(int id)
{
	return engine->input_em->GetKey(id) == KEY_REPEAT;
}
static bool GetKeyboardKeyUp(int id)
{
	return engine->input_em->GetKey(id) == KEY_UP;
}
static bool GetKeyboardKeyIdle(int id)
{
	return engine->input_em->GetKey(id) == KEY_IDLE;
}
static bool GetMouseButtonDown(int id)
{
	return engine->input_em->GetMouseButtonDown(id) == KEY_DOWN;
}
static bool GetMouseButtonRepeat(int id)
{
	return engine->input_em->GetMouseButtonDown(id) == KEY_REPEAT;
}
static bool GetMouseButtonUp(int id)
{
	return engine->input_em->GetMouseButtonDown(id) == KEY_UP;
}
static bool GetMouseButtonIdle(int id)
{
	return engine->input_em->GetMouseButtonDown(id) == KEY_IDLE;
}
static vec2f GetMousePosition(int id)
{
	int x, y;
	engine->input_em->GetMousePosition(x, y);
	return vec2f(x, y);
}
static vec2f GetMouseMotion(int id)
{
	int x, y;
	engine->input_em->GetMouseMotion(x, y);
	return vec2f(x, y);
}
static int GetMouseWheelMotion(int id)
{
	return engine->input_em->GetMouseWheelMotion();
}
static void CloseApp()
{
	engine->input_em->CloseApp();
}


static void LoadScene(MonoString* path)
{
	//for the moment, NON async
	engine->scene_manager_em->LoadSceneFromJson(MonoRegisterer::ToCppString(path));
}
static void CreateEmptyGO(GameObject* go)
{
	engine->scene_manager_em->GetCurrentScene().CreateEmptyGO(go ? *go : engine->scene_manager_em->GetCurrentScene().GetSceneRoot());
}
static void DuplicateGO(GameObject* go)
{
	if (!go) return;
	engine->scene_manager_em->GetCurrentScene().DuplicateGO(*go);
}
static void DeleteGO(GameObject* go)
{
	if (!go) return;
	engine->scene_manager_em->GetCurrentScene().SafeDeleteGO(go->GetSharedPtr());
}
static GameObject* FindGameObjectByID(unsigned int id)
{
	return engine->scene_manager_em->GetCurrentScene().FindGameObjectByID(id).get();
}
static GameObject* FindGameObjectByName(MonoString* name)
{
	return engine->scene_manager_em->GetCurrentScene().FindGameObjectByName(MonoRegisterer::ToCppString(name)).get();
}
static GameObject* GetSceneRoot()
{
	return &engine->scene_manager_em->GetCurrentScene().GetSceneRoot();
}
static GameObject* GetCurrentCameraGO()
{
	if (!engine->scene_manager_em->GetCurrentScene().HasCameraSet()) return nullptr;
	return &engine->scene_manager_em->GetCurrentScene().GetCurrentCameraGO();
}
static void SetCurrentCameraGO(GameObject* go)
{
	if (!go) return;
	engine->scene_manager_em->GetCurrentScene().SetCurrentCameraGO(go->GetSharedPtr());
}


static float GetDT()
{
	return (float)engine->GetDT();
}

void ScriptingLog(MonoString* log, ...)
{
	std::string format = MonoRegisterer::ToCppString(log);

	va_list args;
	va_start(args, log);

	char formattedMsg[BUFFER_SIZE];
	vsnprintf(formattedMsg, sizeof(formattedMsg), format.c_str(), args);
	va_end(args);

	LOG(LogType::LOG_MONO, "%s", formattedMsg);
}

static bool IsComponentEnabled(GameObject* go, int component_type)
{
	if (!go) return false;
	switch ((ComponentType)component_type)
	{
	case ComponentType::Transform:
	{
		if (auto c = go->GetComponent<Transform>())
			return c->IsEnabled();
		break;
	}
	case ComponentType::Camera:
	{
		if (auto c = go->GetComponent<Camera>())
			return c->IsEnabled();
		break;
	}
	case ComponentType::Sprite:
	{
		if (auto c = go->GetComponent<Sprite2D>())
			return c->IsEnabled();
		break;
	}
	case ComponentType::Animator:
	{
		if (auto c = go->GetComponent<Animator>())
			return c->IsEnabled();
		break;
	}
	case ComponentType::Collider2D:
	{
		if (auto c = go->GetComponent<Collider2D>())
			return c->IsEnabled();
		break;
	}
	case ComponentType::Canvas:
	{
		if (auto c = go->GetComponent<Canvas>())
			return c->IsEnabled();
		break;
	}
	case ComponentType::AudioSource:
	{
		if (auto c = go->GetComponent<AudioSource>())
			return c->IsEnabled();
		break;
	}
	case ComponentType::ParticleSystem:
	{
		if (auto c = go->GetComponent<ParticleSystem>())
			return c->IsEnabled();
		break;
	}
	case ComponentType::TileMap:
	{
		if (auto c = go->GetComponent<TileMap>())
			return c->IsEnabled();
		break;
	}
	case ComponentType::Unkown:
		break;
	default:
		break;
	}
}
static void SetEnableComponent(GameObject* go, int component_type, bool enable)
{
	if (!go) return ;
	switch ((ComponentType)component_type)
	{
	case ComponentType::Transform:
	{
		if (auto c = go->GetComponent<Transform>())
			c->SetEnabled(enable);
		break;
	}
	case ComponentType::Camera:
	{
		if (auto c = go->GetComponent<Camera>())
			c->SetEnabled(enable);
		break;
	}
	case ComponentType::Sprite:
	{
		if (auto c = go->GetComponent<Sprite2D>())
			c->SetEnabled(enable);
		break;
	}
	case ComponentType::Animator:
	{
		if (auto c = go->GetComponent<Animator>())
			c->SetEnabled(enable);
		break;
	}
	case ComponentType::Collider2D:
	{
		if (auto c = go->GetComponent<Collider2D>())
			c->SetEnabled(enable);
		break;
	}
	case ComponentType::Canvas:
	{
		if (auto c = go->GetComponent<Canvas>())
			c->SetEnabled(enable);
		break;
	}
	case ComponentType::AudioSource:
	{
		if (auto c = go->GetComponent<AudioSource>())
			c->SetEnabled(enable);
		break;
	}
	case ComponentType::ParticleSystem:
	{
		if (auto c = go->GetComponent<ParticleSystem>())
			c->SetEnabled(enable);
		break;
	}
	case ComponentType::TileMap:
	{
		if (auto c = go->GetComponent<TileMap>())
			c->SetEnabled(enable);
		break;
	}
	case ComponentType::Unkown:
		break;
	default:
		break;
	}
}

static vec2f GetLocalPosition(GameObject* go)
{
	if (!go) return { 0.f, 0.0f };
	if (auto t = go->GetComponent<Transform>())
		return t->GetPosition();
	return { 0.f, 0.0f };
}
static void SetLocalPosition(GameObject* go, vec2f pos)
{
	if (!go) return;
	if (auto t = go->GetComponent<Transform>())
		t->SetPosition(pos);
}
static float GetLocalAngle(GameObject* go)
{
	if (!go) return 0;
	if (auto t = go->GetComponent<Transform>())
		return t->GetAngle();
	return 0;
}
static void SetLocalAngle(GameObject* go, float angle)
{
	if (!go) return;
	if (auto t = go->GetComponent<Transform>())
		t->SetAngle(angle);
}
static vec2f GetLocalScale(GameObject* go)
{
	if (!go) return { 0.f, 0.0f };
	if (auto t = go->GetComponent<Transform>())
		return t->GetScale();
	return { 0.f, 0.0f };
}
static void SetLocalScale(GameObject* go, vec2f scale)
{
	if (!go) return;
	if (auto t = go->GetComponent<Transform>())
		t->SetScale(scale);
}
static vec2f GetWorldPosition(GameObject* go)
{
	if (!go) return { 0.f, 0.0f };
	if (auto t = go->GetComponent<Transform>())
		return t->GetWorldPosition();
	return { 0.f, 0.0f };
}
static void SetWorldPosition(GameObject* go, vec2f pos)
{
	if (!go) return;
	if (auto t = go->GetComponent<Transform>())
		t->SetWorldPosition(pos);
}
static float GetWorldAngle(GameObject* go)
{
	if (!go) return 0;
	if (auto t = go->GetComponent<Transform>())
		return t->GetWorldAngle();
	return 0;
}
static void SetWorldAngle(GameObject* go, float angle)
{
	if (!go) return;
	if (auto t = go->GetComponent<Transform>())
		t->SetWorldAngle(angle);
}
static vec2f GetWorldScale(GameObject* go)
{
	if (!go) return { 0.f, 0.0f };
	if (auto t = go->GetComponent<Transform>())
		return t->GetWorldScale();
	return { 0.f, 0.0f };
}
static void SetWorldScale(GameObject* go, vec2f scale)
{
	if (!go) return;
	if (auto t = go->GetComponent<Transform>())
		t->SetWorldScale(scale);
}
static bool IsAspectRatioLocked(GameObject* go)
{
	if (!go) return false;
	if (auto t = go->GetComponent<Transform>())
		return t->IsAspectRatioLocked();
	return false;
}
static void SetAspectRatioLock(GameObject* go, bool lock)
{
	if (!go) return;
	if (auto t = go->GetComponent<Transform>())
		t->SetAspectRatioLock(lock);
}



static vec2f GetViewportSize(GameObject* go)
{
	int w = 0, h = 0;
	if (!go) return { w, h };
	if (auto c = go->GetComponent<Camera>())
		c->GetViewportSize(w, h);
	return { w, h };
}
static void SetViewportSize(GameObject* go, vec2f size)
{
	if (!go) return;
	if (auto c = go->GetComponent<Camera>())
		c->SetViewportSize(size.x, size.y);
}
static float GetZoom(GameObject* go)
{
	if (!go) return 0;
	if (auto c = go->GetComponent<Camera>())
		return c->GetZoom();
	return 0;
}
static void SetZoom(GameObject* go, float zoom)
{
	if (!go) return;
	if (auto c = go->GetComponent<Camera>())
		c->SetZoom(zoom);
}



static vec2f GetTextureSize(GameObject* go)
{
	int w = 0, h = 0;
	if (!go) return { w, h };
	if (auto c = go->GetComponent<Sprite2D>())
		c->GetTextureSize(w, h);
	return { w, h };
}
static vec4f GetTextureSection(GameObject* go)
{
	ML_Rect r;
	if (!go) return { r.x, r.y, r.w, r.h };
	if (auto c = go->GetComponent<Sprite2D>())
		ML_Rect r = c->GetTextureSection();
	return { r.x, r.y, r.w, r.h };
}
static void SetTextureSection(GameObject* go, vec4f section)
{
	if (!go) return;
	if (auto c = go->GetComponent<Sprite2D>())
		c->SetTextureSection(section.x, section.y, section.z, section.w);
}
static vec2f GetOffset(GameObject* go)
{
	int w = 0, h = 0;
	if (!go) return { w, h };
	if (auto c = go->GetComponent<Sprite2D>())
		return c->GetOffset();
	return { w, h };
}
static void SetOffset(GameObject* go, vec2f offset)
{
	if (!go) return;
	if (auto c = go->GetComponent<Sprite2D>())
		c->SetOffset(offset);
}
static bool IsPixelArt(GameObject* go)
{
	if (!go) return false;
	if (auto c = go->GetComponent<Sprite2D>())
		return c->IsPixelArt();
	return false;
}
static void SetPixelArtRender(GameObject* go, bool pixel_art)
{
	if (!go) return;
	if (auto c = go->GetComponent<Sprite2D>())
		c->SetPixelArtRender(pixel_art);
}
static void SwapTexture(GameObject* go, MonoString* path)
{
	if (!go) return;
	if (auto c = go->GetComponent<Sprite2D>())
		c->SwapTexture(MonoRegisterer::ToCppString(path));
}


static void PlayAnimation(GameObject* go, MonoString* animation)
{
	if (!go) return;
	if (auto c = go->GetComponent<Animator>())
		c->Play(MonoRegisterer::ToCppString(animation));
}
static void StopAnimation(GameObject* go)
{
	if (!go) return;
	if (auto c = go->GetComponent<Animator>())
		c->Stop();
}
static bool IsAnimaitonPlaying(GameObject* go, MonoString* animation)
{
	if (!go) return false;
	if (auto c = go->GetComponent<Animator>())
		return c->IsPlaying(MonoRegisterer::ToCppString(animation));
	return false;
}


static void PauseCollider2D(GameObject* go)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->Pause();
}
static void UnpauseCollider2D(GameObject* go)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->Unpause();
}
static int GetShapeType(GameObject* go)
{
	if (!go) return 0;
	if (auto c = go->GetComponent<Collider2D>())
		return (int)c->GetShapeType();
	return 0;
}
static void SetShapeType(GameObject* go, int shapeType)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetShapeType((ShapeType)shapeType);
}
static vec2f GetColliderSize(GameObject* go)
{
	int w = 0, h = 0;
	if (!go) return { w, h };
	if (auto c = go->GetComponent<Collider2D>())
	{
		w = c->GetWidth();
		h = c->GetHeight();
	}
	return { w, h };
}
static void SetColliderSize(GameObject* go, vec2f size)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetSize(size.x, size.y);
}
static float GetColliderRadius(GameObject* go)
{
	if (!go) return 0;
	if (auto c = go->GetComponent<Collider2D>())
		return c->GetRadius();
	return 0;
}
static void SetColliderRadius(GameObject* go, float radius)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetRadius(radius);
}
static bool IsCollierDynamic(GameObject* go)
{
	if (!go) return false;
	if (auto c = go->GetComponent<Collider2D>())
		return c->IsDynamic();
	return false;
}
static void SetColliderDynamic(GameObject* go, bool dynamic)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetDynamic(dynamic);
}
static bool IsCollierSensor(GameObject* go)
{
	if (!go) return false;
	if (auto c = go->GetComponent<Collider2D>())
		return c->IsSensor();
	return false;
}
static void SetColliderSensor(GameObject* go, bool sensor)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetSensor(sensor);
}
static bool IsCollierRotationLocked(GameObject* go)
{
	if (!go) return false;
	if (auto c = go->GetComponent<Collider2D>())
		return c->GetLockRotation();
	return false;
}
static void SetCollierRotationLocked(GameObject* go, bool lock)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetLockRotation(lock);
}
static bool IsCollierUsingGravity(GameObject* go)
{
	if (!go) return false;
	if (auto c = go->GetComponent<Collider2D>())
		return c->GetUseGravity();
	return false;
}
static void SetCollierUsingGravity(GameObject* go, bool grav)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetUseGravity(grav);
}
static void ApplyForceCollider(GameObject* go, vec2f f)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->ApplyForce(f.x, f.y);
}
static vec2f GetSpeedCollider(GameObject* go)
{
	if (!go) return { 0, 0 };
	if (auto c = go->GetComponent<Collider2D>())
		return c->GetVelocity();
	return { 0, 0 };
}
static void SetSpeedCollider(GameObject* go, vec2f s)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetVelocity(s.x, s.y);
}
static float GetColliderFriction(GameObject* go)
{
	if (!go) return 0;
	if (auto c = go->GetComponent<Collider2D>())
		return c->GetFriction();
	return 0;
}
static void SetColliderFriction(GameObject* go, float f)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetFriction(f);
}
static float GetColliderLinearDamping(GameObject* go)
{
	if (!go) return 0;
	if (auto c = go->GetComponent<Collider2D>())
		return c->GetLinearDamping();
	return 0;
}
static void SetColliderLinearDamping(GameObject* go, float ld)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetLinearDamping(ld);
}
static float GetColliderMass(GameObject* go)
{
	if (!go) return 0;
	if (auto c = go->GetComponent<Collider2D>())
		return c->GetMass();
	return 0;
}
static void SetColliderMass(GameObject* go, float mass)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetMass(mass);
}
static float GetColliderRestitution(GameObject* go)
{
	if (!go) return 0;
	if (auto c = go->GetComponent<Collider2D>())
		return c->GetRestitution();
	return 0;
}
static void SetColliderRestitution(GameObject* go, float r)
{
	if (!go) return;
	if (auto c = go->GetComponent<Collider2D>())
		c->SetRestitution(r);
}


static int GetStateButtonUI(GameObject* go, MonoString* item_ui)
{
	if (!go) return 0;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::ButtonImage)
			{
				if (item->GetName() == item_name)
				{
					return (int)c->GetUIElement<ButtonImageUI>(item->GetID())->GetButtonSectionManager().button_state;
				}
			}
		}
	}
	return 0;
}
static void SetStateButtonUI(GameObject* go, MonoString* item_ui, int state)
{
	if (!go) return;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::ButtonImage)
			{
				if (item->GetName() == item_name)
				{
					c->GetUIElement<ButtonImageUI>(item->GetID())->GetButtonSectionManager().button_state = (ButtonState)state;
					return;
				}
			}
		}
	}
}
static int GetStateCheckboxUI(GameObject* go, MonoString* item_ui)
{
	if (!go) return 0;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::CheckBox)
			{
				if (item->GetName() == item_name)
				{
					return (int)c->GetUIElement<CheckBoxUI>(item->GetID())->GetSectionManager().checkbox_state;
				}
			}
		}
	}
	return 0;
}
static void SetStateCheckboxUI(GameObject* go, MonoString* item_ui, int state)
{
	if (!go) return;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::CheckBox)
			{
				if (item->GetName() == item_name)
				{
					c->GetUIElement<CheckBoxUI>(item->GetID())->GetSectionManager().checkbox_state = (CheckBoxState)state;
					return;
				}
			}
		}
	}
}
static bool GetValueCheckboxUI(GameObject* go, MonoString* item_ui)
{
	if (!go) return 0;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::CheckBox)
			{
				if (item->GetName() == item_name)
				{
					return (bool)c->GetUIElement<CheckBoxUI>(item->GetID())->GetValue();
				}
			}
		}
	}
	return 0;
}
static void SetValueCheckboxUI(GameObject* go, MonoString* item_ui, bool value)
{
	if (!go) return;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::CheckBox)
			{
				if (item->GetName() == item_name)
				{
					c->GetUIElement<CheckBoxUI>(item->GetID())->SetValue(value);
					return;
				}
			}
		}
	}
}
static int GetStateSliderUI(GameObject* go, MonoString* item_ui)
{
	if (!go) return 0;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::Slider)
			{
				if (item->GetName() == item_name)
				{
					return (int)c->GetUIElement<SliderUI>(item->GetID())->GetSliderState();
				}
			}
		}
	}
	return 0;
}
static void SetStateSliderUI(GameObject* go, MonoString* item_ui, int state)
{
	if (!go) return;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::Slider)
			{
				if (item->GetName() == item_name)
				{
					c->GetUIElement<SliderUI>(item->GetID())->SetSliderState((SliderState)state);
					return;
				}
			}
		}
	}
}
static int GetValueSliderUI(GameObject* go, MonoString* item_ui)
{
	if (!go) return 0;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::Slider)
			{
				if (item->GetName() == item_name)
				{
					return (int)c->GetUIElement<SliderUI>(item->GetID())->GetValue();
				}
			}
		}
	}
	return 0;
}
static void SetValueSliderUI(GameObject* go, MonoString* item_ui, int value)
{
	if (!go) return;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::Slider)
			{
				if (item->GetName() == item_name)
				{
					c->GetUIElement<SliderUI>(item->GetID())->SetValue(value);
					return;
				}
			}
		}
	}
}
static MonoString* GetTextTextUI(GameObject* go, MonoString* item_ui)
{
	if (!go) return 0;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::Text)
			{
				if (item->GetName() == item_name)
				{
					return MonoRegisterer::ToMonoString(c->GetUIElement<TextUI>(item->GetID())->GetText());
				}
			}
		}
	}
	return 0;
}
static void SetTextTextUI(GameObject* go, MonoString* item_ui, MonoString* text)
{
	if (!go) return;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetType() == UIElementType::Text)
			{
				if (item->GetName() == item_name)
				{
					c->GetUIElement<TextUI>(item->GetID())->SetText(MonoRegisterer::ToCppString(text));
					return;
				}
			}
		}
	}
}
static vec2f GetItemUIPosition(GameObject* go, MonoString* item_ui)
{
	if (!go) return { 0, 0};
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetName() == item_name)
			{
				return item->GetPosition();
			}
		}
	}
	return { 0, 0 };
}
static void SetItemUIPosition(GameObject* go, MonoString* item_ui, vec2f position)
{
	if (!go) return;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetName() == item_name)
			{
				item->SetPosition(position);
				return;
			}
		}
	}
}
static float GetItemUIAngle(GameObject* go, MonoString* item_ui)
{
	if (!go) return 0;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetName() == item_name)
			{
				return item->GetAngle();
			}
		}
	}
	return 0;
}
static void SetItemUIAngle(GameObject* go, MonoString* item_ui, float angle)
{
	if (!go) return;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetName() == item_name)
			{
				item->SetAngle(angle);
				return;
			}
		}
	}
}
static vec2f GetItemUIScale(GameObject* go, MonoString* item_ui)
{
	if (!go) return { 0, 0 };
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetName() == item_name)
			{
				return item->GetScale();
			}
		}
	}
	return { 0, 0 };
}
static void SetItemUIScale(GameObject* go, MonoString* item_ui, vec2f s)
{
	if (!go) return;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetName() == item_name)
			{
				item->SetScale(s);
				return;
			}
		}
	}
}
static bool GetItemUIAspectLock(GameObject* go, MonoString* item_ui)
{
	if (!go) return 0;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetName() == item_name)
			{
				return item->GetLockedAspectRatio();
			}
		}
	}
	return 0;
}
static void SetItemUIAspectLock(GameObject* go, MonoString* item_ui, bool l)
{
	if (!go) return;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetName() == item_name)
			{
				item->SetAspectLocked(l);
				return;
			}
		}
	}
}
static bool IsEnabledItemUI(GameObject* go, MonoString* item_ui)
{
	if (!go) return 0;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetName() == item_name)
			{
				return item->IsEnabled();
			}
		}
	}
	return 0;
}
static void EnableItemUI(GameObject* go, MonoString* item_ui, bool e)
{
	if (!go) return;
	std::string item_name = MonoRegisterer::ToCppString(item_ui);
	if (auto c = go->GetComponent<Canvas>())
	{
		for (const auto& item : c->GetUIElements())
		{
			if (item->GetName() == item_name)
			{
				item->SetEnabled(e);
				return;
			}
		}
	}
}


static void PauseAudioSource(GameObject* go)
{
	if (!go) return;
	if (auto c = go->GetComponent<AudioSource>())
		c->PauseAll();
}
static void UnpauseAudioSource(GameObject* go)
{
	if (!go) return;
	if (auto c = go->GetComponent<AudioSource>())
		c->UnpauseAll();
}
static void StopAudioSource(GameObject* go)
{
	if (!go) return;
	if (auto c = go->GetComponent<AudioSource>())
		c->StopAll();
}
static void PlayMusic(GameObject* go, MonoString* audio)
{
	if (!go) return;
	if (auto c = go->GetComponent<AudioSource>())
		c->PlayMusic(MonoRegisterer::ToCppString(audio));
}
static void PlaySound(GameObject* go, MonoString* audio)
{
	if (!go) return;
	if (auto c = go->GetComponent<AudioSource>())
		c->PlaySound(MonoRegisterer::ToCppString(audio));
}
static void PauseMusic(GameObject* go, MonoString* audio)
{
	if (!go) return;
	if (auto c = go->GetComponent<AudioSource>())
		c->PauseMusic(MonoRegisterer::ToCppString(audio));
}
static void PauseSound(GameObject* go, MonoString* audio)
{
	if (!go) return;
	if (auto c = go->GetComponent<AudioSource>())
		c->PauseSound(MonoRegisterer::ToCppString(audio));
}
static void StopMusic(GameObject* go, MonoString* audio)
{
	if (!go) return;
	if (auto c = go->GetComponent<AudioSource>())
		c->StopMusic(MonoRegisterer::ToCppString(audio));
}
static void StopSound(GameObject* go, MonoString* audio)
{
	if (!go) return;
	if (auto c = go->GetComponent<AudioSource>())
		c->StopSound(MonoRegisterer::ToCppString(audio));
}
static int GetMusicVolume(GameObject* go, MonoString* audio)
{
	if (!go) return 0;
	if (auto c = go->GetComponent<AudioSource>())
		return c->GetMusicVolume(MonoRegisterer::ToCppString(audio));
	return 0;
}
static int GetSoundVolume(GameObject* go, MonoString* audio)
{
	if (!go) return 0;
	if (auto c = go->GetComponent<AudioSource>())
		return c->GetSoundVolume(MonoRegisterer::ToCppString(audio));
	return 0;
}
static void SetMusicVolume(GameObject* go, MonoString* audio, int volume)
{
	if (!go) return;
	if (auto c = go->GetComponent<AudioSource>())
		c->SetMusicVolume(MonoRegisterer::ToCppString(audio), volume);
}
static void SetSoundVolume(GameObject* go, MonoString* audio, int volume)
{
	if (!go) return;
	if (auto c = go->GetComponent<AudioSource>())
		c->SetSoundVolume(MonoRegisterer::ToCppString(audio), volume);
}


static void PlayParticleSystem(GameObject* go)
{
	if (!go) return;
	if (auto c = go->GetComponent<ParticleSystem>())
		c->Init();
}
static void PauseParticleSystem(GameObject* go)
{
	if (!go) return;
	if (auto c = go->GetComponent<ParticleSystem>())
		c->Pause();
}
static void UnpauseParticleSystem(GameObject* go)
{
	if (!go) return;
	if (auto c = go->GetComponent<ParticleSystem>())
		c->Unpause();
}
static void StopParticleSystem(GameObject* go)
{
	if (!go) return;
	if (auto c = go->GetComponent<ParticleSystem>())
		c->Stop();
}


void MonoBehaviour_Ctor(MonoObject* self)
{

}


void MonoRegisterer::RegisterFunctions()
{
	//go
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetGOPtr", (void*)GetGOPtr);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetGOName", (void*)GetGOName);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetGOName", (void*)SetGOName);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetGOTag", (void*)GetGOTag);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetGOTag", (void*)SetGOTag);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetGOID", (void*)GetGOID);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetGOParent", (void*)GetGOParent);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetGOEnabled", (void*)GetGOEnabled);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetGOEnabled", (void*)SetGOEnabled);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SwitchGOEnabled", (void*)SwitchGOEnabled);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetGOVisible", (void*)GetGOVisible);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetGOVisible", (void*)SetGOVisible);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SwitchGOVisible", (void*)SwitchGOVisible);
	mono_add_internal_call("ManLiteScripting.InternalCalls::ReparentGO", (void*)ReparentGO);

	//input
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetKeyboardKeyDown", (void*)GetKeyboardKeyDown);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetKeyboardKeyRepeat", (void*)GetKeyboardKeyRepeat);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetKeyboardKeyUp", (void*)GetKeyboardKeyUp);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetKeyboardKeyIdle", (void*)GetKeyboardKeyIdle);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetMouseButtonDown", (void*)GetMouseButtonDown);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetMouseButtonRepeat", (void*)GetMouseButtonRepeat);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetMouseButtonUp", (void*)GetMouseButtonUp);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetMouseButtonIdle", (void*)GetMouseButtonIdle);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetMousePosition", (void*)GetMousePosition);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetMouseMotion", (void*)GetMouseMotion);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetMouseWheelMotion", (void*)GetMouseWheelMotion);
	mono_add_internal_call("ManLiteScripting.InternalCalls::CloseApp", (void*)CloseApp);

	//scene manager and scene
	mono_add_internal_call("ManLiteScripting.InternalCalls::LoadScene", (void*)LoadScene);
	mono_add_internal_call("ManLiteScripting.InternalCalls::CreateEmptyGO", (void*)CreateEmptyGO);
	mono_add_internal_call("ManLiteScripting.InternalCalls::DeleteGO", (void*)DeleteGO);
	mono_add_internal_call("ManLiteScripting.InternalCalls::FindGameObjectByID", (void*)FindGameObjectByID);
	mono_add_internal_call("ManLiteScripting.InternalCalls::FindGameObjectByName", (void*)FindGameObjectByName);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetSceneRoot", (void*)GetSceneRoot);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetCurrentCameraGO", (void*)GetCurrentCameraGO);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetCurrentCameraGO", (void*)SetCurrentCameraGO);

	//utils
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetDT", (void*)GetDT);
	mono_add_internal_call("ManLiteScripting.InternalCalls::ScriptingLog", (void*)ScriptingLog);

	//components
	mono_add_internal_call("ManLiteScripting.InternalCalls::IsComponentEnabled", (void*)IsComponentEnabled);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetEnableComponent", (void*)SetEnableComponent);
	//transform
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetLocalPosition", (void*)GetLocalPosition);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetLocalPosition", (void*)SetLocalPosition);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetLocalAngle", (void*)GetLocalAngle);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetLocalAngle", (void*)SetLocalAngle);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetLocalScale", (void*)GetLocalScale);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetLocalScale", (void*)SetLocalScale);

	mono_add_internal_call("ManLiteScripting.InternalCalls::GetWorldPosition", (void*)GetWorldPosition);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetWorldPosition", (void*)SetWorldPosition);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetWorldAngle", (void*)GetWorldAngle);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetWorldAngle", (void*)SetWorldAngle);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetWorldScale", (void*)GetWorldScale);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetWorldScale", (void*)SetWorldScale);
	mono_add_internal_call("ManLiteScripting.InternalCalls::IsAspectRatioLocked", (void*)IsAspectRatioLocked);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetAspectRatioLock", (void*)SetAspectRatioLock);
	
	//camera
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetViewportSize", (void*)GetViewportSize);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetViewportSize", (void*)SetViewportSize);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetZoom", (void*)GetZoom);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetZoom", (void*)SetZoom);

	//sprite
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetTextureSize", (void*)GetTextureSize);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetTextureSection", (void*)GetTextureSection);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetTextureSection", (void*)SetTextureSection);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetOffset", (void*)GetOffset);
	mono_add_internal_call("ManLiteScripting.InternalCalls::IsPixelArt", (void*)IsPixelArt);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetPixelArtRender", (void*)SetPixelArtRender);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SwapTexture", (void*)SwapTexture);

	//animator
	mono_add_internal_call("ManLiteScripting.InternalCalls::PlayAnimation", (void*)PlayAnimation);
	mono_add_internal_call("ManLiteScripting.InternalCalls::PlayAnimation", (void*)StopAnimation);
	mono_add_internal_call("ManLiteScripting.InternalCalls::PlayAnimation", (void*)IsAnimaitonPlaying);

	//collider2d
	mono_add_internal_call("ManLiteScripting.InternalCalls::PauseCollider2D", (void*)PauseCollider2D);
	mono_add_internal_call("ManLiteScripting.InternalCalls::UnpauseCollider2D", (void*)UnpauseCollider2D);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetShapeType", (void*)GetShapeType);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetShapeType", (void*)SetShapeType);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetColliderSize", (void*)GetColliderSize);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetColliderSize", (void*)SetColliderSize);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetColliderRadius", (void*)GetColliderRadius);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetColliderRadius", (void*)SetColliderRadius);
	mono_add_internal_call("ManLiteScripting.InternalCalls::IsCollierDynamic", (void*)IsCollierDynamic);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetColliderDynamic", (void*)SetColliderDynamic);
	mono_add_internal_call("ManLiteScripting.InternalCalls::IsCollierSensor", (void*)IsCollierSensor);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetColliderSensor", (void*)SetColliderSensor);
	mono_add_internal_call("ManLiteScripting.InternalCalls::IsCollierRotationLocked", (void*)IsCollierRotationLocked);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetCollierRotationLocked", (void*)SetCollierRotationLocked);
	mono_add_internal_call("ManLiteScripting.InternalCalls::IsCollierUsingGravity", (void*)IsCollierUsingGravity);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetCollierUsingGravity", (void*)SetCollierUsingGravity);
	mono_add_internal_call("ManLiteScripting.InternalCalls::ApplyForceCollider", (void*)ApplyForceCollider);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetSpeedCollider", (void*)GetSpeedCollider);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetSpeedCollider", (void*)SetSpeedCollider);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetColliderFriction", (void*)GetColliderFriction);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetColliderFriction", (void*)SetColliderFriction);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetColliderLinearDamping", (void*)GetColliderLinearDamping);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetColliderLinearDamping", (void*)SetColliderLinearDamping);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetColliderMass", (void*)GetColliderMass);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetColliderMass", (void*)SetColliderMass);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetColliderRestitution", (void*)GetColliderRestitution);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetColliderRestitution", (void*)SetColliderRestitution);

	//canvas
	//button
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetStateButtonUI", (void*)GetStateButtonUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetStateButtonUI", (void*)SetStateButtonUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetStateCheckboxUI", (void*)GetStateCheckboxUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetStateCheckboxUI", (void*)SetStateCheckboxUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetValueCheckboxUI", (void*)GetValueCheckboxUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetValueCheckboxUI", (void*)SetValueCheckboxUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetStateSliderUI", (void*)GetStateSliderUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetStateSliderUI", (void*)SetStateSliderUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetValueSliderUI", (void*)GetValueSliderUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetValueSliderUI", (void*)SetValueSliderUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetTextTextUI", (void*)GetTextTextUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetTextTextUI", (void*)SetTextTextUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetItemUIPosition", (void*)GetItemUIPosition);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetItemUIPosition", (void*)SetItemUIPosition);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetItemUIAngle", (void*)GetItemUIAngle);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetItemUIAngle", (void*)SetItemUIAngle);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetItemUIScale", (void*)GetItemUIScale);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetItemUIScale", (void*)SetItemUIScale);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetItemUIAspectLock", (void*)GetItemUIAspectLock);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetItemUIAspectLock", (void*)SetItemUIAspectLock);
	mono_add_internal_call("ManLiteScripting.InternalCalls::IsEnabledItemUI", (void*)IsEnabledItemUI);
	mono_add_internal_call("ManLiteScripting.InternalCalls::EnableItemUI", (void*)EnableItemUI);

	//audio
	mono_add_internal_call("ManLiteScripting.InternalCalls::PauseAudioSource", (void*)PauseAudioSource);
	mono_add_internal_call("ManLiteScripting.InternalCalls::UnpauseAudioSource", (void*)UnpauseAudioSource);
	mono_add_internal_call("ManLiteScripting.InternalCalls::StopAudioSource", (void*)StopAudioSource);
	mono_add_internal_call("ManLiteScripting.InternalCalls::PlayMusic", (void*)PlayMusic);
	mono_add_internal_call("ManLiteScripting.InternalCalls::PlaySound", (void*)PlaySound);
	mono_add_internal_call("ManLiteScripting.InternalCalls::PauseMusic", (void*)PauseMusic);
	mono_add_internal_call("ManLiteScripting.InternalCalls::PauseSound", (void*)PauseSound);
	mono_add_internal_call("ManLiteScripting.InternalCalls::StopMusic", (void*)StopMusic);
	mono_add_internal_call("ManLiteScripting.InternalCalls::StopSound", (void*)StopSound);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetMusicVolume", (void*)GetMusicVolume);
	mono_add_internal_call("ManLiteScripting.InternalCalls::GetSoundVolume", (void*)GetSoundVolume);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetMusicVolume", (void*)SetMusicVolume);
	mono_add_internal_call("ManLiteScripting.InternalCalls::SetSoundVolume", (void*)SetSoundVolume);

	//particles
	mono_add_internal_call("ManLiteScripting.InternalCalls::PlayParticleSystem", (void*)PlayParticleSystem);
	mono_add_internal_call("ManLiteScripting.InternalCalls::PauseParticleSystem", (void*)PauseParticleSystem);
	mono_add_internal_call("ManLiteScripting.InternalCalls::UnpauseParticleSystem", (void*)UnpauseParticleSystem);
	mono_add_internal_call("ManLiteScripting.InternalCalls::StopParticleSystem", (void*)StopParticleSystem);

	//others
	mono_add_internal_call("ManLiteScripting.ManLiteScripting.MonoBehaviour::.ctor", (void*)MonoBehaviour_Ctor);
}

MonoString* MonoRegisterer::ToMonoString(const std::string& str)
{
	return mono_string_new(ScriptingEM::GetAppDomain(), str.c_str());
}

std::string MonoRegisterer::ToCppString(MonoString* monoStr)
{
    if (!monoStr) return "";

    char* utf8Str = mono_string_to_utf8(monoStr);
    std::string result(utf8Str);
    mono_free(utf8Str);

    return result;
}
