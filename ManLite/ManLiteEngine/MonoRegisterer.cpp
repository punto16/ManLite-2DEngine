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
	int w, h = 0;
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
	int w, h = 0;
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
	int w, h = 0;
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
	int w, h = 0;
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


void MonoRegisterer::RegisterFunctions()
{
	//go
	mono_add_internal_call("InternalCalls::GetGOPtr", GetGOPtr);
	mono_add_internal_call("InternalCalls::GetGOName", GetGOName);
	mono_add_internal_call("InternalCalls::SetGOName", SetGOName);
	mono_add_internal_call("InternalCalls::GetGOTag", GetGOTag);
	mono_add_internal_call("InternalCalls::SetGOTag", SetGOTag);
	mono_add_internal_call("InternalCalls::GetGOID", GetGOID);
	mono_add_internal_call("InternalCalls::GetGOParent", GetGOParent);
	mono_add_internal_call("InternalCalls::GetGOEnabled", GetGOEnabled);
	mono_add_internal_call("InternalCalls::SetGOEnabled", SetGOEnabled);
	mono_add_internal_call("InternalCalls::SwitchGOEnabled", SwitchGOEnabled);
	mono_add_internal_call("InternalCalls::GetGOVisible", GetGOVisible);
	mono_add_internal_call("InternalCalls::SetGOVisible", SetGOVisible);
	mono_add_internal_call("InternalCalls::SwitchGOVisible", SwitchGOVisible);
	mono_add_internal_call("InternalCalls::ReparentGO", ReparentGO);

	//input
	mono_add_internal_call("InternalCalls::GetKeyboardKeyDown", GetKeyboardKeyDown);
	mono_add_internal_call("InternalCalls::GetKeyboardKeyRepeat", GetKeyboardKeyRepeat);
	mono_add_internal_call("InternalCalls::GetKeyboardKeyUp", GetKeyboardKeyUp);
	mono_add_internal_call("InternalCalls::GetKeyboardKeyIdle", GetKeyboardKeyIdle);
	mono_add_internal_call("InternalCalls::GetMouseButtonDown", GetMouseButtonDown);
	mono_add_internal_call("InternalCalls::GetMouseButtonRepeat", GetMouseButtonRepeat);
	mono_add_internal_call("InternalCalls::GetMouseButtonUp", GetMouseButtonUp);
	mono_add_internal_call("InternalCalls::GetMouseButtonIdle", GetMouseButtonIdle);
	mono_add_internal_call("InternalCalls::GetMousePosition", GetMousePosition);
	mono_add_internal_call("InternalCalls::GetMouseMotion", GetMouseMotion);
	mono_add_internal_call("InternalCalls::GetMouseWheelMotion", GetMouseWheelMotion);
	mono_add_internal_call("InternalCalls::CloseApp", CloseApp);

	//scene manager and scene
	mono_add_internal_call("InternalCalls::LoadScene", LoadScene);
	mono_add_internal_call("InternalCalls::CreateEmptyGO", CreateEmptyGO);
	mono_add_internal_call("InternalCalls::DeleteGO", DeleteGO);
	mono_add_internal_call("InternalCalls::FindGameObjectByID", FindGameObjectByID);
	mono_add_internal_call("InternalCalls::FindGameObjectByName", FindGameObjectByName);
	mono_add_internal_call("InternalCalls::GetSceneRoot", GetSceneRoot);
	mono_add_internal_call("InternalCalls::GetCurrentCameraGO", GetCurrentCameraGO);
	mono_add_internal_call("InternalCalls::SetCurrentCameraGO", SetCurrentCameraGO);

	//utils
	mono_add_internal_call("InternalCalls::GetDT", GetDT);

	//components
	mono_add_internal_call("InternalCalls::IsComponentEnabled", IsComponentEnabled);
	mono_add_internal_call("InternalCalls::SetEnableComponent", SetEnableComponent);
	//transform
	mono_add_internal_call("InternalCalls::GetLocalPosition", GetLocalPosition);
	mono_add_internal_call("InternalCalls::SetLocalPosition", SetLocalPosition);
	mono_add_internal_call("InternalCalls::GetLocalAngle", GetLocalAngle);
	mono_add_internal_call("InternalCalls::SetLocalAngle", SetLocalAngle);
	mono_add_internal_call("InternalCalls::GetLocalScale", GetLocalScale);
	mono_add_internal_call("InternalCalls::SetLocalScale", SetLocalScale);

	mono_add_internal_call("InternalCalls::GetWorldPosition", GetWorldPosition);
	mono_add_internal_call("InternalCalls::SetWorldPosition", SetWorldPosition);
	mono_add_internal_call("InternalCalls::GetWorldAngle", GetWorldAngle);
	mono_add_internal_call("InternalCalls::SetWorldAngle", SetWorldAngle);
	mono_add_internal_call("InternalCalls::GetWorldScale", GetWorldScale);
	mono_add_internal_call("InternalCalls::SetWorldScale", SetWorldScale);
	mono_add_internal_call("InternalCalls::IsAspectRatioLocked", IsAspectRatioLocked);
	mono_add_internal_call("InternalCalls::SetAspectRatioLock", SetAspectRatioLock);
	
	//camera
	mono_add_internal_call("InternalCalls::GetViewportSize", GetViewportSize);
	mono_add_internal_call("InternalCalls::SetViewportSize", SetViewportSize);
	mono_add_internal_call("InternalCalls::GetZoom", GetZoom);
	mono_add_internal_call("InternalCalls::SetZoom", SetZoom);

	//sprite
	mono_add_internal_call("InternalCalls::GetTextureSize", GetTextureSize);
	mono_add_internal_call("InternalCalls::GetTextureSection", GetTextureSection);
	mono_add_internal_call("InternalCalls::SetTextureSection", SetTextureSection);
	mono_add_internal_call("InternalCalls::GetOffset", GetOffset);
	mono_add_internal_call("InternalCalls::IsPixelArt", IsPixelArt);
	mono_add_internal_call("InternalCalls::SetPixelArtRender", SetPixelArtRender);
	mono_add_internal_call("InternalCalls::SwapTexture", SwapTexture);

	//animator
	mono_add_internal_call("InternalCalls::PlayAnimation", PlayAnimation);
	mono_add_internal_call("InternalCalls::PlayAnimation", StopAnimation);
	mono_add_internal_call("InternalCalls::PlayAnimation", IsAnimaitonPlaying);

	//collider2d
	mono_add_internal_call("InternalCalls::PauseCollider2D", PauseCollider2D);
	mono_add_internal_call("InternalCalls::UnpauseCollider2D", UnpauseCollider2D);
	mono_add_internal_call("InternalCalls::GetShapeType", GetShapeType);
	mono_add_internal_call("InternalCalls::SetShapeType", SetShapeType);
	mono_add_internal_call("InternalCalls::GetColliderSize", GetColliderSize);
	mono_add_internal_call("InternalCalls::SetColliderSize", SetColliderSize);
	mono_add_internal_call("InternalCalls::GetColliderRadius", GetColliderRadius);
	mono_add_internal_call("InternalCalls::SetColliderRadius", SetColliderRadius);
	mono_add_internal_call("InternalCalls::IsCollierDynamic", IsCollierDynamic);
	mono_add_internal_call("InternalCalls::SetColliderDynamic", SetColliderDynamic);
	mono_add_internal_call("InternalCalls::IsCollierSensor", IsCollierSensor);
	mono_add_internal_call("InternalCalls::SetColliderSensor", SetColliderSensor);
	mono_add_internal_call("InternalCalls::IsCollierRotationLocked", IsCollierRotationLocked);
	mono_add_internal_call("InternalCalls::SetCollierRotationLocked", SetCollierRotationLocked);
	mono_add_internal_call("InternalCalls::IsCollierUsingGravity", IsCollierUsingGravity);
	mono_add_internal_call("InternalCalls::SetCollierUsingGravity", SetCollierUsingGravity);
	mono_add_internal_call("InternalCalls::ApplyForceCollider", ApplyForceCollider);
	mono_add_internal_call("InternalCalls::GetSpeedCollider", GetSpeedCollider);
	mono_add_internal_call("InternalCalls::SetSpeedCollider", SetSpeedCollider);
	mono_add_internal_call("InternalCalls::GetColliderFriction", GetColliderFriction);
	mono_add_internal_call("InternalCalls::SetColliderFriction", SetColliderFriction);
	mono_add_internal_call("InternalCalls::GetColliderLinearDamping", GetColliderLinearDamping);
	mono_add_internal_call("InternalCalls::SetColliderLinearDamping", SetColliderLinearDamping);
	mono_add_internal_call("InternalCalls::GetColliderMass", GetColliderMass);
	mono_add_internal_call("InternalCalls::SetColliderMass", SetColliderMass);
	mono_add_internal_call("InternalCalls::GetColliderRestitution", GetColliderRestitution);
	mono_add_internal_call("InternalCalls::SetColliderRestitution", SetColliderRestitution);

	//canvas
	//button
	mono_add_internal_call("InternalCalls::GetStateButtonUI", GetStateButtonUI);
	mono_add_internal_call("InternalCalls::SetStateButtonUI", SetStateButtonUI);
	mono_add_internal_call("InternalCalls::GetStateCheckboxUI", GetStateCheckboxUI);
	mono_add_internal_call("InternalCalls::SetStateCheckboxUI", SetStateCheckboxUI);
	mono_add_internal_call("InternalCalls::GetValueCheckboxUI", GetValueCheckboxUI);
	mono_add_internal_call("InternalCalls::SetValueCheckboxUI", SetValueCheckboxUI);
	mono_add_internal_call("InternalCalls::GetStateSliderUI", GetStateSliderUI);
	mono_add_internal_call("InternalCalls::SetStateSliderUI", SetStateSliderUI);
	mono_add_internal_call("InternalCalls::GetValueSliderUI", GetValueSliderUI);
	mono_add_internal_call("InternalCalls::SetValueSliderUI", SetValueSliderUI);
	mono_add_internal_call("InternalCalls::GetTextTextUI", GetTextTextUI);
	mono_add_internal_call("InternalCalls::SetTextTextUI", SetTextTextUI);
	mono_add_internal_call("InternalCalls::GetItemUIPosition", GetItemUIPosition);
	mono_add_internal_call("InternalCalls::SetItemUIPosition", SetItemUIPosition);
	mono_add_internal_call("InternalCalls::GetItemUIAngle", GetItemUIAngle);
	mono_add_internal_call("InternalCalls::SetItemUIAngle", SetItemUIAngle);
	mono_add_internal_call("InternalCalls::GetItemUIScale", GetItemUIScale);
	mono_add_internal_call("InternalCalls::SetItemUIScale", SetItemUIScale);
	mono_add_internal_call("InternalCalls::GetItemUIAspectLock", GetItemUIAspectLock);
	mono_add_internal_call("InternalCalls::SetItemUIAspectLock", SetItemUIAspectLock);
	mono_add_internal_call("InternalCalls::IsEnabledItemUI", IsEnabledItemUI);
	mono_add_internal_call("InternalCalls::EnableItemUI", EnableItemUI);

	//audio
	mono_add_internal_call("InternalCalls::PauseAudioSource", PauseAudioSource);
	mono_add_internal_call("InternalCalls::UnpauseAudioSource", UnpauseAudioSource);
	mono_add_internal_call("InternalCalls::StopAudioSource", StopAudioSource);
	mono_add_internal_call("InternalCalls::PlayMusic", PlayMusic);
	mono_add_internal_call("InternalCalls::PlaySound", PlaySound);
	mono_add_internal_call("InternalCalls::PauseMusic", PauseMusic);
	mono_add_internal_call("InternalCalls::PauseSound", PauseSound);
	mono_add_internal_call("InternalCalls::StopMusic", StopMusic);
	mono_add_internal_call("InternalCalls::StopSound", StopSound);
	mono_add_internal_call("InternalCalls::GetMusicVolume", GetMusicVolume);
	mono_add_internal_call("InternalCalls::GetSoundVolume", GetSoundVolume);
	mono_add_internal_call("InternalCalls::SetMusicVolume", SetMusicVolume);
	mono_add_internal_call("InternalCalls::SetSoundVolume", SetSoundVolume);

	//particles
	mono_add_internal_call("InternalCalls::PlayParticleSystem", PlayParticleSystem);
	mono_add_internal_call("InternalCalls::PauseParticleSystem", PauseParticleSystem);
	mono_add_internal_call("InternalCalls::UnpauseParticleSystem", UnpauseParticleSystem);
	mono_add_internal_call("InternalCalls::StopParticleSystem", StopParticleSystem);
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
