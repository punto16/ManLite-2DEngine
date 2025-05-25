using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ManLiteScripting
{
    public class IGameObject
    {
        private IntPtr _game_object_ptr = IntPtr.Zero;

        public IntPtr game_object_ptr
        {
            get
            {
                if (_game_object_ptr == IntPtr.Zero)
                    _game_object_ptr = InternalCalls.GetGOPtr();
                return _game_object_ptr;
            }
            private set
            {
                _game_object_ptr = value;
            }
        }

        public bool IsValid => game_object_ptr != IntPtr.Zero;

        public string name
        {
            get
            {
                return InternalCalls.GetGOName(game_object_ptr);
            }
            set
            {
                InternalCalls.SetGOName(game_object_ptr, value);
            }
        }

        public bool enabled
        {
            get
            {
                return InternalCalls.GetGOEnabled(game_object_ptr);
            }
            set
            {
                InternalCalls.SetGOEnabled(game_object_ptr, value);
            }
        }

        public bool visible
        {
            get
            {
                return InternalCalls.GetGOVisible(game_object_ptr);
            }
            set
            {
                InternalCalls.SetGOVisible(game_object_ptr, value);
            }
        }

        public string tag
        {
            get
            {
                return InternalCalls.GetGOTag(game_object_ptr);
            }
            set
            {
                InternalCalls.SetGOTag(game_object_ptr, value);
            }
        }

        public uint id
        {
            get
            {
                return InternalCalls.GetGOID(game_object_ptr);
            }
        }

        public IGameObject GetParentGO()
        {
            IGameObject parentGO = new IGameObject(InternalCalls.GetGOParent(game_object_ptr));
            return parentGO;
        }
        public void ReparentGO(IGameObject new_parent)
        {
            InternalCalls.ReparentGO(game_object_ptr, new_parent.game_object_ptr);
        }

        public IGameObject()
        {
            game_object_ptr = InternalCalls.GetGOPtr();
        }

        public IGameObject(IntPtr go)
        {
            game_object_ptr = go;
        }

        public static implicit operator bool(IGameObject go)
        {
            return go?.IsValid ?? false;
        }

        public bool Exists()
        {
            return (bool)this;
        }

        public static bool operator ==(IGameObject a, IGameObject b) => a?.game_object_ptr == b?.game_object_ptr;
        public static bool operator !=(IGameObject a, IGameObject b) => !(a == b);
        public override bool Equals(object obj) => obj is IGameObject other && this == other;
        public override int GetHashCode() => game_object_ptr.GetHashCode();
    }

    public class Input
    {
        public Input() { }

        public static KeyState GetKeyboardKey(KeyboardKey key)
        {
            if (InternalCalls.GetKeyboardKeyDown((int)key)) return KeyState.KEY_DOWN;
            if (InternalCalls.GetKeyboardKeyRepeat((int)key)) return KeyState.KEY_REPEAT;
            if (InternalCalls.GetKeyboardKeyDown((int)key)) return KeyState.KEY_DOWN;
            return KeyState.KEY_IDLE;
        }

        public static KeyState GetMouseButton(MouseButton button)
        {
            if (InternalCalls.GetMouseButtonDown((int)button)) return KeyState.KEY_DOWN;
            if (InternalCalls.GetMouseButtonRepeat((int)button)) return KeyState.KEY_REPEAT;
            if (InternalCalls.GetMouseButtonDown((int)button)) return KeyState.KEY_DOWN;
            return KeyState.KEY_IDLE;
        }

        public static Vec2f GetMousePosition()
        {
            Vec2f mouse_pos = new Vec2f();
            InternalCalls.GetMousePosition(out mouse_pos);
            return mouse_pos;
        }

        public static Vec2f GetMouseMotion()
        {
            Vec2f mouse_motion = new Vec2f();
            InternalCalls.GetMouseMotion(out mouse_motion);
            return mouse_motion;
        }

        public static int GetMouseWheelMotion()
        {
            return InternalCalls.GetMouseWheelMotion();
        }

        public static void CloseApp()
        {
            InternalCalls.CloseApp();
        }
    }

    public class Scene
    {
        public Scene() { }

        public static void LoadScene(string path)
        {
            InternalCalls.LoadScene(path);
        }

        public static void LoadSceneAsync(string path, bool set_scene_on_finish_load)
        {
            InternalCalls.LoadSceneBackGround(path, set_scene_on_finish_load);
        }

        public static void SetBackGroundLoadedScene()
        {
            InternalCalls.SetBackGroundLoadedScene();
        }

        public static IGameObject CreateEmptyGO(IGameObject parent, bool root_is_parent = false)
        {
            if (root_is_parent) return new IGameObject(InternalCalls.CreateEmptyGO(InternalCalls.GetSceneRoot()));
            else return new IGameObject(InternalCalls.CreateEmptyGO(parent.game_object_ptr));
        }

        public static IGameObject DuplicateGO(IGameObject go)
        {
            return new IGameObject(InternalCalls.DuplicateGO(go.game_object_ptr));
        }

        public static void DeleteGO(IGameObject go)
        {
            InternalCalls.DeleteGO(go.game_object_ptr);
        }

        public static IGameObject FindGameObjectByID(uint id)
        {
            return new IGameObject(InternalCalls.FindGameObjectByID(id));
        }

        public static IGameObject FindGameObjectByName(string name)
        {
            return new IGameObject(InternalCalls.FindGameObjectByName(name));
        }

        public static IGameObject GetSceneRoot()
        {
            return new IGameObject(InternalCalls.GetSceneRoot());
        }

        public static IGameObject GetCurrentCameraGO()
        {
            return new IGameObject(InternalCalls.GetCurrentCameraGO());
        }

        public static void SetCurrentCameraGO(IGameObject new_cam_go)
        {
            InternalCalls.SetCurrentCameraGO(new_cam_go.game_object_ptr);
        }

        public static IGameObject InstantiatePrefab(string path)
        {
            return new IGameObject(InternalCalls.InstantiatePrefab(path));
        }
    }

    public class ML_System
    {
        public ML_System() { }

        public static float GetDT()
        {
            return InternalCalls.GetDT();
        }

        public static void Log(string format, params object[] args)
        {
            string message = args.Length > 0 ? string.Format(format, args) : format;
            InternalCalls.ScriptingLog(message, __arglist());
        }
    }

    public class Component
    {
        private IGameObject container_go;

        public Component(IGameObject go)
        {
            container_go = go;
        }

        public bool IsComponentEnabled(ComponentType type)
        {
            return InternalCalls.IsComponentEnabled(container_go.game_object_ptr, (int)type);
        }

        public void EnableComponent(ComponentType type)
        {
            InternalCalls.SetEnableComponent(container_go.game_object_ptr, (int)type, true);
        }

        public void DisableComponent(ComponentType type)
        {
            InternalCalls.SetEnableComponent(container_go.game_object_ptr, (int)type, false);
        }
    }

    public class Transform
    {
        public IGameObject container_go;

        public Transform(IGameObject go)
        {
            container_go = go;
        }

        //local transform
        public Vec2f GetLocalPosition()
        {
            Vec2f pos = new Vec2f();
            InternalCalls.GetLocalPosition(container_go.game_object_ptr, out pos);
            return pos;
        }

        public void SetLocalPosition(Vec2f pos)
        {
            InternalCalls.SetLocalPosition(container_go.game_object_ptr, pos);
        }

        public float GetLocalAngle()
        {
            return InternalCalls.GetLocalAngle(container_go.game_object_ptr);
        }

        public void SetLocalAngle(float angle)
        {
            InternalCalls.SetLocalAngle(container_go.game_object_ptr, angle);
        }

        public Vec2f GetLocalScale()
        {
            Vec2f scale = new Vec2f();
            InternalCalls.GetLocalScale(container_go.game_object_ptr, out scale);
            return scale;
        }

        public void SetLocalScale(Vec2f scale)
        {
            InternalCalls.SetLocalScale(container_go.game_object_ptr, scale);
        }

        //world transform
        public Vec2f GetWorldPosition()
        {
            Vec2f pos = new Vec2f();
            InternalCalls.GetWorldPosition(container_go.game_object_ptr, out pos);
            return pos;
        }

        public void SetWorldPosition(Vec2f pos)
        {
            InternalCalls.SetWorldPosition(container_go.game_object_ptr, pos);
        }

        public float GetWorldAngle()
        {
            return InternalCalls.GetWorldAngle(container_go.game_object_ptr);
        }

        public void SetWorldAngle(float angle)
        {
            InternalCalls.SetWorldAngle(container_go.game_object_ptr, angle);
        }

        public Vec2f GetWorldScale()
        {
            Vec2f scale = new Vec2f();
            InternalCalls.GetWorldScale(container_go.game_object_ptr, out scale);
            return scale;
        }

        public void SetWorldScale(Vec2f scale)
        {
            InternalCalls.SetWorldScale(container_go.game_object_ptr, scale);
        }

        //scale aspect lock
        public bool IsAspectRatioLocked()
        {
            return InternalCalls.IsAspectRatioLocked(container_go.game_object_ptr);
        }

        public void SetAspectRatioLock(bool b)
        {
            InternalCalls.SetAspectRatioLock(container_go.game_object_ptr, b);
        }
    }

    public class Camera
    {
        public IGameObject container_go;

        public Camera(IGameObject go)
        {
            container_go = go;
        }

        public Vec2f GetViewportSize()
        {
            Vec2f size = new Vec2f();
            InternalCalls.GetViewportSize(container_go.game_object_ptr, out size);
            return size;
        }

        public void SetViewportSize(Vec2f size)
        {
            InternalCalls.SetViewportSize(container_go.game_object_ptr, size);
        }

        public float GetZoom()
        {
            return InternalCalls.GetZoom(container_go.game_object_ptr);
        }

        public void SetZoom(float zoom)
        {
            InternalCalls.SetZoom(container_go.game_object_ptr, zoom);
        }
    }

    public class Sprite
    {
        public IGameObject container_go;

        public Sprite(IGameObject go)
        {
            container_go = go;
        }

        public Vec2f GetTextureSize()
        {
            Vec2f size = new Vec2f();
            InternalCalls.GetTextureSize(container_go.game_object_ptr, out size);
            return size;
        }

        public Vec4f GetTextureSection()
        {
            Vec4f section = new Vec4f();
            InternalCalls.GetTextureSection(container_go.game_object_ptr, out section);
            return section;
        }

        public void SetTextureSection(Vec4f section)
        {
            InternalCalls.SetTextureSection(container_go.game_object_ptr, section);
        }

        public Vec2f GetTextureOffset()
        {
            Vec2f offset = new Vec2f();
            InternalCalls.GetOffset(container_go.game_object_ptr, out offset);
            return offset;
        }

        public void SetTextureOffset(Vec2f offset)
        {
            InternalCalls.SetOffset(container_go.game_object_ptr, offset);
        }

        public bool IsPixelArt()
        {
            return InternalCalls.IsPixelArt(container_go.game_object_ptr);
        }

        public void SetPixelArt(bool b)
        {
            InternalCalls.SetPixelArtRender(container_go.game_object_ptr, b);
        }

        public void SwapTexture(string path)
        {
            InternalCalls.SwapTexture(container_go.game_object_ptr, path);
        }
    }

    public class Animator
    {
        public IGameObject container_go;

        public Animator(IGameObject go)
        {
            container_go = go;
        }

        public void PlayAnimation(string name)
        {
            InternalCalls.PlayAnimation(container_go.game_object_ptr, name);
        }

        public void StopAnimation()
        {
            InternalCalls.StopAnimation(container_go.game_object_ptr);
        }

        public bool IsAnimationPlaying(string name)
        {
            return InternalCalls.IsAnimationPlaying(container_go.game_object_ptr, name);
        }
    }

    public class Collider2D
    {
        public IGameObject container_go;

        public Collider2D(IGameObject go)
        {
            container_go = go;
        }

        public void Pause()
        {
            InternalCalls.PauseCollider2D(container_go.game_object_ptr);
        }

        public void Unpause()
        {
            InternalCalls.UnpauseCollider2D(container_go.game_object_ptr);
        }

        public ShapeType GetShapeType()
        {
            return (ShapeType)InternalCalls.GetShapeType(container_go.game_object_ptr);
        }

        public void SetShapeType(ShapeType type)
        {
            InternalCalls.SetShapeType(container_go.game_object_ptr, (int)type);
        }

        public Vec2f GetSize()
        {
            Vec2f size = new Vec2f();
            InternalCalls.GetColliderSize(container_go.game_object_ptr, out size);
            return size;
        }

        public void SetSize(Vec2f size)
        {
            InternalCalls.SetColliderSize(container_go.game_object_ptr, size);
        }

        public float GetRadius()
        {
            return InternalCalls.GetColliderRadius(container_go.game_object_ptr);
        }

        public void GetRadius(float radius)
        {
            InternalCalls.SetColliderRadius(container_go.game_object_ptr, radius);
        }

        public bool IsDynamic()
        {
            return InternalCalls.IsCollierDynamic(container_go.game_object_ptr);
        }

        public void SetDynamic(bool dynamic)
        {
            InternalCalls.SetColliderDynamic(container_go.game_object_ptr, dynamic);
        }

        public bool IsSensor()
        {
            return InternalCalls.IsCollierSensor(container_go.game_object_ptr);
        }

        public void SetSensor(bool sensor)
        {
            InternalCalls.SetColliderSensor(container_go.game_object_ptr, sensor);
        }

        public bool IsRotatinLocked()
        {
            return InternalCalls.IsCollierRotationLocked(container_go.game_object_ptr);
        }

        public void SetRotationLock(bool r)
        {
            InternalCalls.SetCollierRotationLocked(container_go.game_object_ptr, r);
        }

        public bool IsUsingGravity()
        {
            return InternalCalls.IsCollierUsingGravity(container_go.game_object_ptr);
        }

        public void SetGravityUse(bool g)
        {
            InternalCalls.SetCollierUsingGravity(container_go.game_object_ptr, g);
        }

        public void ApplyForce(Vec2f force)
        {
            InternalCalls.ApplyForceCollider(container_go.game_object_ptr, force);
        }

        public Vec2f GetSpeed()
        {
            Vec2f speed = new Vec2f();
            InternalCalls.GetSpeedCollider(container_go.game_object_ptr, out speed);
            return speed;
        }

        public void SetSpeed(Vec2f speed)
        {
            InternalCalls.SetSpeedCollider(container_go.game_object_ptr, speed);
        }

        public float GetFriction()
        {
            return InternalCalls.GetColliderFriction(container_go.game_object_ptr);
        }

        public void SetFriction(float friction)
        {
            InternalCalls.SetColliderFriction(container_go.game_object_ptr, friction);
        }

        public float GetLinearDamping()
        {
            return InternalCalls.GetColliderLinearDamping(container_go.game_object_ptr);
        }

        public void SetLinearDamping(float l)
        {
            InternalCalls.SetColliderLinearDamping(container_go.game_object_ptr, l);
        }

        public float GetMass()
        {
            return InternalCalls.GetColliderMass(container_go.game_object_ptr);
        }

        public void SetMass(float m)
        {
            InternalCalls.SetColliderMass(container_go.game_object_ptr, m);
        }

        public float GetRestitution()
        {
            return InternalCalls.GetColliderRestitution(container_go.game_object_ptr);
        }

        public void SetRestitution(float r)
        {
            InternalCalls.SetColliderRestitution(container_go.game_object_ptr, r);
        }
    }

    public class Canvas
    {
        public IGameObject container_go;

        public Canvas(IGameObject go)
        {
            container_go = go;
        }

        public ButtonState GetButtonState(string name)
        {
            return (ButtonState)InternalCalls.GetStateButtonUI(container_go.game_object_ptr, name);
        }

        public void SetButtonState(string name, ButtonState state)
        {
            InternalCalls.SetStateButtonUI(container_go.game_object_ptr, name, (int)state);
        }

        public CheckboxState GetCheckboxState(string name)
        {
            return (CheckboxState)InternalCalls.GetStateCheckboxUI(container_go.game_object_ptr, name);
        }

        public void SetCheckboxState(string name, CheckboxState state)
        {
            InternalCalls.SetStateCheckboxUI(container_go.game_object_ptr, name, (int)state);
        }

        public bool GetCheckboxValue(string name)
        {
            return InternalCalls.GetValueCheckboxUI(container_go.game_object_ptr, name);
        }

        public void SetCheckboxValue(string name, bool v)
        {
            InternalCalls.SetValueCheckboxUI(container_go.game_object_ptr, name, v);
        }

        public SliderState GetSliderState(string name)
        {
            return (SliderState)InternalCalls.GetStateSliderUI(container_go.game_object_ptr, name);
        }

        public void SetSliderState(string name, SliderState state)
        {
            InternalCalls.SetStateSliderUI(container_go.game_object_ptr, name, (int)state);
        }

        public int GetSliderValue(string name)
        {
            return InternalCalls.GetValueSliderUI(container_go.game_object_ptr, name);
        }

        public void SetSliderValue(string name, int v)
        {
            InternalCalls.SetValueSliderUI(container_go.game_object_ptr, name, v);
        }

        public string GetText(string name)
        {
            return InternalCalls.GetTextTextUI(container_go.game_object_ptr, name);
        }

        public void SetText(string name, string text)
        {
            InternalCalls.SetTextTextUI(container_go.game_object_ptr, name, text);
        }

        public Vec2f GetPosition(string name)
        {
            Vec2f pos = new Vec2f();
            InternalCalls.GetItemUIPosition(container_go.game_object_ptr, name, out pos);
            return pos;
        }

        public void SetPosition(string name, Vec2f pos)
        {
            InternalCalls.SetItemUIPosition(container_go.game_object_ptr, name, pos);
        }

        public float GetAngle(string name)
        {
            return InternalCalls.GetItemUIAngle(container_go.game_object_ptr, name);
        }

        public void SetAngle(string name, float angle)
        {
            InternalCalls.SetItemUIAngle(container_go.game_object_ptr, name, angle);
        }

        public Vec2f GetScale(string name)
        {
            Vec2f scale = new Vec2f();
            InternalCalls.GetItemUIScale(container_go.game_object_ptr, name, out scale);
            return scale;
        }

        public void SetScale(string name, Vec2f scale)
        {
            InternalCalls.SetItemUIScale(container_go.game_object_ptr, name, scale);
        }

        public bool IsAspectRatioLocked(string name)
        {
            return InternalCalls.GetItemUIAspectLock(container_go.game_object_ptr, name);
        }

        public void SetAspectRatioLock(string name, bool b)
        {
            InternalCalls.SetItemUIAspectLock(container_go.game_object_ptr, name, b);
        }

        public bool IsItemUIEnabled(string name)
        {
            return InternalCalls.IsEnabledItemUI(container_go.game_object_ptr, name);
        }

        public void SetItemUIEnabled(string name, bool b)
        {
            InternalCalls.EnableItemUI(container_go.game_object_ptr, name, b);
        }
    }

    public class Audio
    {
        public IGameObject container_go;

        public Audio(IGameObject go)
        {
            container_go = go;
        }

        public void PauseAll()
        {
            InternalCalls.PauseAudioSource(container_go.game_object_ptr);
        }

        public void UnpauseAll()
        {
            InternalCalls.UnpauseAudioSource(container_go.game_object_ptr);
        }

        public void StopAll()
        {
            InternalCalls.StopAudioSource(container_go.game_object_ptr);
        }

        public void PlayMusic(string audio)
        {
            InternalCalls.PlayMusic(container_go.game_object_ptr, audio);
        }

        public void PlaySound(string audio)
        {
            InternalCalls.PlaySound(container_go.game_object_ptr, audio);
        }

        public void PauseMusic(string audio)
        {
            InternalCalls.PauseMusic(container_go.game_object_ptr, audio);
        }

        public void PauseSound(string audio)
        {
            InternalCalls.PauseSound(container_go.game_object_ptr, audio);
        }

        public void StopMusic(string audio)
        {
            InternalCalls.StopMusic(container_go.game_object_ptr, audio);
        }

        public void StopSound(string audio)
        {
            InternalCalls.StopSound(container_go.game_object_ptr, audio);
        }

        public int GetMusicVolume(string audio)
        {
            return InternalCalls.GetMusicVolume(container_go.game_object_ptr, audio);
        }

        public int GetSoundVolume(string audio)
        {
            return InternalCalls.GetSoundVolume(container_go.game_object_ptr, audio);
        }

        public void SetMusicVolume(string audio, int volume)
        {
            InternalCalls.SetMusicVolume(container_go.game_object_ptr, audio, volume);
        }

        public void SetSoundVolume(string audio, int volume)
        {
            InternalCalls.SetSoundVolume(container_go.game_object_ptr, audio, volume);
        }
    }

    public class Particles
    {
        public IGameObject container_go;

        public Particles(IGameObject go)
        {
            container_go = go;
        }

        public void Play()
        {
            InternalCalls.PlayParticleSystem(container_go.game_object_ptr);
        }

        public void Pause()
        {
            InternalCalls.PauseParticleSystem(container_go.game_object_ptr);
        }

        public void Unpause()
        {
            InternalCalls.UnpauseParticleSystem(container_go.game_object_ptr);
        }

        public void Stop()
        {
            InternalCalls.StopParticleSystem(container_go.game_object_ptr);
        }
    }

    public class Light
    {
        public IGameObject container_go;

        public Light(IGameObject go)
        {
            container_go = go;
        }

        public void SetColor(Vec4f color)
        {
            InternalCalls.SetLightColor(container_go.game_object_ptr, color);
        }

        public Vec4f GetColor()
        {
            Vec4f color = new Vec4f();
            InternalCalls.GetLightColor(container_go.game_object_ptr, out color);
            return color;
        }

        public void SetIntensity(float intensity)
        {
            InternalCalls.SetLightIntensity(container_go.game_object_ptr, intensity);
        }

        public float GetIntensity()
        {
            return InternalCalls.GetLightIntensity(container_go.game_object_ptr);
        }

        public void SetRadius(float r)
        {
            InternalCalls.SetLightRadius(container_go.game_object_ptr, r);
        }

        public float GetRadius()
        {
            return InternalCalls.GetLightRadius(container_go.game_object_ptr);
        }

        public void SetEndRadius(float r)
        {
            InternalCalls.SetLightEndRadius(container_go.game_object_ptr, r);
        }

        public float GetEndRadius()
        {
            return InternalCalls.GetLightEndRadius(container_go.game_object_ptr);
        }

        public void SetEndPosition(Vec2f pos)
        {
            InternalCalls.SetLightEndPosition(container_go.game_object_ptr, pos);
        }

        public Vec2f GetEndPosition()
        {
            Vec2f pos = new Vec2f();
            InternalCalls.GetLightEndPosition(container_go.game_object_ptr, out pos);
            return pos;
        }

        public void SetFinalPositionStatic(bool b)
        {
            InternalCalls.SetLightFinalPosStatic(container_go.game_object_ptr, b);
        }

        public bool GetFinalPositionStatic()
        {
            return InternalCalls.GetLightFinalPosStatic(container_go.game_object_ptr);
        }
    }
}
