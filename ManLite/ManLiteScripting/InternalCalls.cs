using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace ManLiteScripting
{
    #region Structs
    [StructLayout(LayoutKind.Sequential)]
    public struct Vec2f : IEquatable<Vec2f>
    {
        public float X;
        public float Y;

        public Vec2f(float x, float y)
        {
            X = x;
            Y = y;
        }

        public static bool operator ==(Vec2f left, Vec2f right) => left.Equals(right);
        public static bool operator !=(Vec2f left, Vec2f right) => !(left == right);

        public static Vec2f operator +(Vec2f left, Vec2f right) =>
            new Vec2f(left.X + right.X, left.Y + right.Y);

        public static Vec2f operator -(Vec2f left, Vec2f right) =>
            new Vec2f(left.X - right.X, left.Y - right.Y);

        public static Vec2f operator *(Vec2f left, Vec2f right) =>
            new Vec2f(left.X * right.X, left.Y * right.Y);

        public static Vec2f operator *(Vec2f left, float scalar) =>
            new Vec2f(left.X * scalar, left.Y * scalar);

        public static Vec2f operator *(float scalar, Vec2f right) =>
            right * scalar;

        public static Vec2f operator /(Vec2f vector, float scalar)
        {
            if (Math.Abs(scalar) < 1e-8f)
                throw new DivideByZeroException("Scalar cannot be zero");

            float inv = 1.0f / scalar;
            return new Vec2f(vector.X * inv, vector.Y * inv);
        }

        public static Vec2f operator /(Vec2f left, Vec2f right)
        {
            if (Math.Abs(right.X) < 1e-8f || Math.Abs(right.Y) < 1e-8f)
                throw new DivideByZeroException("Vector component cannot be zero");

            return new Vec2f(left.X / right.X, left.Y / right.Y);
        }

        public bool Equals(Vec2f other)
        {
            const float tolerance = 1e-6f;
            return Math.Abs(X - other.X) < tolerance &&
                   Math.Abs(Y - other.Y) < tolerance;
        }

        public override bool Equals(object obj)
        {
            return obj is Vec2f other && Equals(other);
        }

        public float Magnitude
        {
            get => (float)Math.Sqrt(X * X + Y * Y);
        }

        public void Normalize()
        {
            float magnitude = Magnitude;
            if (magnitude > 1e-8f)
            {
                X /= magnitude;
                Y /= magnitude;
            }
            else
            {
                X = 0;
                Y = 0;
            }
        }

        public override int GetHashCode()
        {
            unchecked
            {
                int hash = 17;
                hash = hash * 31 + X.GetHashCode();
                hash = hash * 31 + Y.GetHashCode();
                return hash;
            }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Vec4f : IEquatable<Vec4f>
    {
        public float X;
        public float Y;
        public float Z;
        public float W;

        public Vec4f(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public static bool operator ==(Vec4f left, Vec4f right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(Vec4f left, Vec4f right)
        {
            return !(left == right);
        }

        public static Vec4f operator +(Vec4f left, Vec4f right) =>
            new Vec4f(left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W);

        public static Vec4f operator -(Vec4f left, Vec4f right) =>
            new Vec4f(left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W);

        public static Vec4f operator *(Vec4f left, Vec4f right) =>
            new Vec4f(left.X * right.X, left.Y * right.Y, left.Z * right.Z, left.W * right.W);

        public static Vec4f operator *(Vec4f left, float scalar) =>
            new Vec4f(left.X * scalar, left.Y * scalar, left.Z * scalar, left.W * scalar);

        public static Vec4f operator *(float scalar, Vec4f right) =>
            right * scalar;

        public static Vec4f operator /(Vec4f vector, float scalar)
        {
            if (Math.Abs(scalar) < 1e-8f)
                throw new DivideByZeroException("Scalar cannot be zero");

            float inv = 1.0f / scalar;
            return new Vec4f(vector.X * inv, vector.Y * inv, vector.Z * inv, vector.W * inv);
        }

        public static Vec4f operator /(Vec4f left, Vec4f right)
        {
            if (Math.Abs(right.X) < 1e-8f || Math.Abs(right.Y) < 1e-8f ||
                Math.Abs(right.Z) < 1e-8f || Math.Abs(right.W) < 1e-8f)
                throw new DivideByZeroException("Vector component cannot be zero");

            return new Vec4f(
                left.X / right.X,
                left.Y / right.Y,
                left.Z / right.Z,
                left.W / right.W);
        }

        public bool Equals(Vec4f other)
        {
            const float tolerance = 1e-6f;
            return Math.Abs(X - other.X) < tolerance &&
                   Math.Abs(Y - other.Y) < tolerance &&
                   Math.Abs(Z - other.Z) < tolerance &&
                   Math.Abs(W - other.W) < tolerance;
        }

        public override bool Equals(object obj)
        {
            return obj is Vec4f other && Equals(other);
        }

        public float Magnitude
        {
            get => (float)Math.Sqrt(X * X + Y * Y + Z * Z + W * W);
        }

        public void Normalize()
        {
            float magnitude = Magnitude;
            if (magnitude > 1e-8f)
            {
                X /= magnitude;
                Y /= magnitude;
                Z /= magnitude;
                W /= magnitude;
            }
            else
            {
                X = 0;
                Y = 0;
                Z = 0;
                W = 0;
            }
        }

        public override int GetHashCode()
        {
            unchecked
            {
                int hash = 17;
                hash = hash * 31 + X.GetHashCode();
                hash = hash * 31 + Y.GetHashCode();
                hash = hash * 31 + Z.GetHashCode();
                hash = hash * 31 + W.GetHashCode();
                return hash;
            }
        }
    }
    #endregion

    public class InternalCalls
    {

        #region GameObject
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static IntPtr GetGOPtr();
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static string GetGOName(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetGOName(IntPtr go, string name);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static string GetGOTag(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetGOTag(IntPtr go, string tag);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static uint GetGOID(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static IntPtr GetGOParent(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static IntPtr GetGOChild(IntPtr go, string name);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetGOEnabled(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetGOEnabled(IntPtr go, bool enable);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SwitchGOEnabled(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetGOVisible(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetGOVisible(IntPtr go, bool visible);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SwitchGOVisible(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void ReparentGO(IntPtr go, IntPtr newParent);
        #endregion


        #region Input
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetKeyboardKeyDown(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetKeyboardKeyRepeat(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetKeyboardKeyUp(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetKeyboardKeyIdle(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetMouseButtonDown(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetMouseButtonRepeat(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetMouseButtonUp(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetMouseButtonIdle(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetMousePosition(out Vec2f pos);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetMouseMotion(out Vec2f motion);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static int GetMouseWheelMotion();
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void CloseApp();
        #endregion


        #region Scene Management
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void LoadScene(string path);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void LoadSceneBackGround(string path, bool set_on_finish_loading);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetBackGroundLoadedScene();
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static IntPtr CreateEmptyGO(IntPtr parent);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static IntPtr DuplicateGO(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static IntPtr DeleteGO(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static IntPtr FindGameObjectByID(uint id);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static IntPtr FindGameObjectByName(string name);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static IntPtr GetSceneRoot();
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static IntPtr GetCurrentCameraGO();
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetCurrentCameraGO(IntPtr cameraGO);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static IntPtr InstantiatePrefab(string prefab_path);
        #endregion


        #region Utils
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetDT();
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void ScriptingLog(string format, __arglist);
        #endregion


        #region Components
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool IsComponentEnabled(IntPtr go, int componentType);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetEnableComponent(IntPtr go, int componentType, bool enable);
        #endregion


        #region Transform
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetLocalPosition(IntPtr go, out Vec2f pos);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetLocalPosition(IntPtr go, Vec2f pos);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetLocalAngle(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetLocalAngle(IntPtr go, float angle);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetLocalScale(IntPtr go, out Vec2f scale);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetLocalScale(IntPtr go, Vec2f scale);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetWorldPosition(IntPtr go, out Vec2f pos);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetWorldPosition(IntPtr go, Vec2f pos);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetWorldAngle(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetWorldAngle(IntPtr go, float angle);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetWorldScale(IntPtr go, out Vec2f scale);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetWorldScale(IntPtr go, Vec2f scale);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool IsAspectRatioLocked(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetAspectRatioLock(IntPtr go, bool enable);
        #endregion


        #region Camera
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetViewportSize(IntPtr cameraGO, out Vec2f size);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetViewportSize(IntPtr cameraGO, Vec2f size);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetZoom(IntPtr cameraGO);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetZoom(IntPtr cameraGO, float zoom);
        #endregion


        #region Sprite
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetTextureSize(IntPtr go, out Vec2f size);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetTextureSection(IntPtr go, out Vec4f section);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetTextureSection(IntPtr go, Vec4f section);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetOffset(IntPtr go, out Vec2f offset);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetOffset(IntPtr go, Vec2f offset);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool IsPixelArt(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetPixelArtRender(IntPtr go, bool enable);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SwapTexture(IntPtr go, string path);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void FlipTextureVertical(IntPtr go, bool v);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetFlipTextureVertical(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void FlipTextureHorizontal(IntPtr go, bool h);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetFlipTextureHorizontal(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetDefaultFlipTextureVertical(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetDefaultFlipTextureHorizontal(IntPtr go);
        #endregion


        #region Animator
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void PlayAnimation(IntPtr go, string animation);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void RePlayAnimation(IntPtr go, string animation);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void StopAnimation(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool IsAnimationPlaying(IntPtr go, string animation);
        #endregion


        #region Collider2D
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void PauseCollider2D(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void UnpauseCollider2D(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static int GetShapeType(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetShapeType(IntPtr go, int shapeType);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetColliderSize(IntPtr go, out Vec2f size);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetColliderSize(IntPtr go, Vec2f size);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetColliderRadius(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetColliderRadius(IntPtr go, float radius);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool IsCollierDynamic(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetColliderDynamic(IntPtr go, bool dynamic);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool IsCollierSensor(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetColliderSensor(IntPtr go, bool sensor);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool IsCollierRotationLocked(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetCollierRotationLocked(IntPtr go, bool locked);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool IsCollierUsingGravity(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetCollierUsingGravity(IntPtr go, bool useGravity);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void ApplyForceCollider(IntPtr go, Vec2f force);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetSpeedCollider(IntPtr go, out Vec2f speed);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetPositionCollider(IntPtr go, Vec2f pos);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetSpeedCollider(IntPtr go, Vec2f speed);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetColliderFriction(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetColliderFriction(IntPtr go, float friction);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetColliderLinearDamping(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetColliderLinearDamping(IntPtr go, float damping);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetColliderMass(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetColliderMass(IntPtr go, float mass);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetColliderRestitution(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetColliderRestitution(IntPtr go, float restitution);
        #endregion


        #region Canvas
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static int GetStateButtonUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetStateButtonUI(IntPtr go, string itemUI, int state);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static int GetStateCheckboxUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetStateCheckboxUI(IntPtr go, string itemUI, int state);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetValueCheckboxUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetValueCheckboxUI(IntPtr go, string itemUI, bool value);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static int GetStateSliderUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetStateSliderUI(IntPtr go, string itemUI, int state);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static int GetValueSliderUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetValueSliderUI(IntPtr go, string itemUI, int value);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static string GetTextTextUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetTextTextUI(IntPtr go, string itemUI, string text);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetItemUIPosition(IntPtr go, string itemUI, out Vec2f pos);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetItemUIPosition(IntPtr go, string itemUI, Vec2f position);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetItemUIAngle(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetItemUIAngle(IntPtr go, string itemUI, float angle);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetItemUIScale(IntPtr go, string itemUI, out Vec2f scale);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetItemUIScale(IntPtr go, string itemUI, Vec2f scale);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetItemUIAspectLock(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetItemUIAspectLock(IntPtr go, string itemUI, bool locked);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool IsEnabledItemUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void EnableItemUI(IntPtr go, string itemUI, bool enabled);
        #endregion


        #region Audio
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void PauseAudioSource(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void UnpauseAudioSource(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void StopAudioSource(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void PlayMusic(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void PlaySound(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void PauseMusic(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void PauseSound(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void UnpauseMusic(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void UnpauseSound(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void StopMusic(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void StopSound(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static int GetMusicVolume(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static int GetSoundVolume(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetMusicVolume(IntPtr go, string audio, int volume);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetSoundVolume(IntPtr go, string audio, int volume);
        #endregion


        #region Particles
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void PlayParticleSystem(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void PauseParticleSystem(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void UnpauseParticleSystem(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void StopParticleSystem(IntPtr go);
        #endregion


        #region Lights
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetLightColor(IntPtr go, Vec4f color);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetLightColor(IntPtr go, out Vec4f color);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetLightIntensity(IntPtr go, float intensity);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetLightIntensity(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetLightRadius(IntPtr go, float radius);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetLightRadius(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetLightEndRadius(IntPtr go, float radius);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static float GetLightEndRadius(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetLightEndPosition(IntPtr go, Vec2f pos);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void GetLightEndPosition(IntPtr go, out Vec2f pos);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetLightFinalPosStatic(IntPtr go, bool b);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static bool GetLightFinalPosStatic(IntPtr go);
        #endregion
    }


    public enum ComponentType
    {
        Transform = 0,
        Camera = 1,
        Sprite = 2,
        Animator = 3,
        Script = 4,
        Collider2D = 5,
        Canvas = 6,
        AudioSource = 7,
        ParticleSystem = 8,
        TileMap = 9,
        Light = 10,


        Unkown
    };

    public enum ShapeType
    {   
        RECTANGLE = 0,
        CIRCLE = 1
    };

    public enum ButtonState
    {
        IDLE = 0,
        HOVERED = 1,
        SELECTED = 2,
        HOVEREDSELECTED = 3,
        DISABLED = 4,

        //
        UNKNOWN
    };

    public enum CheckboxState
    {
        IDLE = 0,
        HOVERED = 1,
        SELECTED = 2,
        HOVEREDSELECTED = 3,
        DISABLED = 4,

        //
        UNKNOWN
    };

    public enum SliderState
    {
        IDLE = 0,
        HOVERED = 1,
        DISABLED = 4,

        //
        UNKNOWN
    };

    public enum KeyState
    {
        KEY_IDLE = 0,
        KEY_DOWN = 1,
        KEY_REPEAT = 2,
        KEY_UP = 3,
    };

    public enum MouseButton
    {
        MOUSE_BUTTON_UNKNOWN = 0,
        MOUSE_BUTTON_LEFT = 1,
        MOUSE_BUTTON_MIDDLE = 2,
        MOUSE_BUTTON_RIGHT = 3,
        MOUSE_BUTTON_X1 = 4,
        MOUSE_BUTTON_X2 = 5
    };

    public enum KeyboardKey
    {
        KEY_UNKNOWN = 0,
        KEY_A = 4,
        KEY_B = 5,
        KEY_C = 6,
        KEY_D = 7,
        KEY_E = 8,
        KEY_F = 9,
        KEY_G = 10,
        KEY_H = 11,
        KEY_I = 12,
        KEY_J = 13,
        KEY_K = 14,
        KEY_L = 15,
        KEY_M = 16,
        KEY_N = 17,
        KEY_O = 18,
        KEY_P = 19,
        KEY_Q = 20,
        KEY_R = 21,
        KEY_S = 22,
        KEY_T = 23,
        KEY_U = 24,
        KEY_V = 25,
        KEY_W = 26,
        KEY_X = 27,
        KEY_Y = 28,
        KEY_Z = 29,

        KEY_1 = 30,
        KEY_2 = 31,
        KEY_3 = 32,
        KEY_4 = 33,
        KEY_5 = 34,
        KEY_6 = 35,
        KEY_7 = 36,
        KEY_8 = 37,
        KEY_9 = 38,
        KEY_0 = 39,

        KEY_RETURN = 40,
        KEY_ESCAPE = 41,
        KEY_BACKSPACE = 42,
        KEY_TAB = 43,
        KEY_SPACE = 44,

        KEY_MINUS = 45,
        KEY_EQUALS = 46,
        KEY_LEFTBRACKET = 47,
        KEY_RIGHTBRACKET = 48,
        KEY_BACKSLASH = 49,
        KEY_NONUSHASH = 50,
        KEY_SEMICOLON = 51,
        KEY_APOSTROPHE = 52,
        KEY_GRAVE = 53,
        KEY_COMMA = 54,
        KEY_PERIOD = 55,
        KEY_SLASH = 56,

        KEY_CAPSLOCK = 57,

        KEY_F1 = 58,
        KEY_F2 = 59,
        KEY_F3 = 60,
        KEY_F4 = 61,
        KEY_F5 = 62,
        KEY_F6 = 63,
        KEY_F7 = 64,
        KEY_F8 = 65,
        KEY_F9 = 66,
        KEY_F10 = 67,
        KEY_F11 = 68,
        KEY_F12 = 69,

        KEY_PRINTSCREEN = 70,
        KEY_SCROLLLOCK = 71,
        KEY_PAUSE = 72,
        KEY_INSERT = 73,
        KEY_HOME = 74,
        KEY_PAGEUP = 75,
        KEY_DELETE = 76,
        KEY_END = 77,
        KEY_PAGEDOWN = 78,
        KEY_RIGHT = 79,
        KEY_LEFT = 80,
        KEY_DOWN = 81,
        KEY_UP = 82,

        KEY_NUMLOCKCLEAR = 83,
        KEY_KP_DIVIDE = 84,
        KEY_KP_MULTIPLY = 85,
        KEY_KP_MINUS = 86,
        KEY_KP_PLUS = 87,
        KEY_KP_ENTER = 88,
        KEY_KP_1 = 89,
        KEY_KP_2 = 90,
        KEY_KP_3 = 91,
        KEY_KP_4 = 92,
        KEY_KP_5 = 93,
        KEY_KP_6 = 94,
        KEY_KP_7 = 95,
        KEY_KP_8 = 96,
        KEY_KP_9 = 97,
        KEY_KP_0 = 98,
        KEY_KP_PERIOD = 99,

        KEY_NONUSBACKSLASH = 100,
        KEY_APPLICATION = 101,
        KEY_POWER = 102,

        KEY_KP_EQUALS = 103,
        KEY_F13 = 104,
        KEY_F14 = 105,
        KEY_F15 = 106,
        KEY_F16 = 107,
        KEY_F17 = 108,
        KEY_F18 = 109,
        KEY_F19 = 110,
        KEY_F20 = 111,
        KEY_F21 = 112,
        KEY_F22 = 113,
        KEY_F23 = 114,
        KEY_F24 = 115,
        KEY_EXECUTE = 116,
        KEY_HELP = 117,
        KEY_MENU = 118,
        KEY_SELECT = 119,
        KEY_STOP = 120,
        KEY_AGAIN = 121,
        KEY_UNDO = 122,
        KEY_CUT = 123,
        KEY_COPY = 124,
        KEY_PASTE = 125,
        KEY_FIND = 126,
        KEY_MUTE = 127,
        KEY_VOLUMEUP = 128,
        KEY_VOLUMEDOWN = 129,

        KEY_KP_COMMA = 133,
        KEY_KP_EQUALSAS400 = 134,

        KEY_INTERNATIONAL1 = 135,

        KEY_INTERNATIONAL2 = 136,
        KEY_INTERNATIONAL3 = 137,
        KEY_INTERNATIONAL4 = 138,
        KEY_INTERNATIONAL5 = 139,
        KEY_INTERNATIONAL6 = 140,
        KEY_INTERNATIONAL7 = 141,
        KEY_INTERNATIONAL8 = 142,
        KEY_INTERNATIONAL9 = 143,
        KEY_LANG1 = 144,
        KEY_LANG2 = 145,
        KEY_LANG3 = 146,
        KEY_LANG4 = 147,
        KEY_LANG5 = 148,
        KEY_LANG6 = 149,
        KEY_LANG7 = 150,
        KEY_LANG8 = 151,
        KEY_LANG9 = 152,

        KEY_ALTERASE = 153,
        KEY_SYSREQ = 154,
        KEY_CANCEL = 155,
        KEY_CLEAR = 156,
        KEY_PRIOR = 157,
        KEY_RETURN2 = 158,
        KEY_SEPARATOR = 159,
        KEY_OUT = 160,
        KEY_OPER = 161,
        KEY_CLEARAGAIN = 162,
        KEY_CRSEL = 163,
        KEY_EXSEL = 164,

        KEY_KP_00 = 176,
        KEY_KP_000 = 177,
        KEY_THOUSANDSSEPARATOR = 178,
        KEY_DECIMALSEPARATOR = 179,
        KEY_CURRENCYUNIT = 180,
        KEY_CURRENCYSUBUNIT = 181,
        KEY_KP_LEFTPAREN = 182,
        KEY_KP_RIGHTPAREN = 183,
        KEY_KP_LEFTBRACE = 184,
        KEY_KP_RIGHTBRACE = 185,
        KEY_KP_TAB = 186,
        KEY_KP_BACKSPACE = 187,
        KEY_KP_A = 188,
        KEY_KP_B = 189,
        KEY_KP_C = 190,
        KEY_KP_D = 191,
        KEY_KP_E = 192,
        KEY_KP_F = 193,
        KEY_KP_XOR = 194,
        KEY_KP_POWER = 195,
        KEY_KP_PERCENT = 196,
        KEY_KP_LESS = 197,
        KEY_KP_GREATER = 198,
        KEY_KP_AMPERSAND = 199,
        KEY_KP_DBLAMPERSAND = 200,
        KEY_KP_VERTICALBAR = 201,
        KEY_KP_DBLVERTICALBAR = 202,
        KEY_KP_COLON = 203,
        KEY_KP_HASH = 204,
        KEY_KP_SPACE = 205,
        KEY_KP_AT = 206,
        KEY_KP_EXCLAM = 207,
        KEY_KP_MEMSTORE = 208,
        KEY_KP_MEMRECALL = 209,
        KEY_KP_MEMCLEAR = 210,
        KEY_KP_MEMADD = 211,
        KEY_KP_MEMSUBTRACT = 212,
        KEY_KP_MEMMULTIPLY = 213,
        KEY_KP_MEMDIVIDE = 214,
        KEY_KP_PLUSMINUS = 215,
        KEY_KP_CLEAR = 216,
        KEY_KP_CLEARENTRY = 217,
        KEY_KP_BINARY = 218,
        KEY_KP_OCTAL = 219,
        KEY_KP_DECIMAL = 220,
        KEY_KP_HEXADECIMAL = 221,

        KEY_LCTRL = 224,
        KEY_LSHIFT = 225,
        KEY_LALT = 226,
        KEY_LGUI = 227,
        KEY_RCTRL = 228,
        KEY_RSHIFT = 229,
        KEY_RALT = 230,
        KEY_RGUI = 231,

        KEY_MODE = 257,

        KEY_AUDIONEXT = 258,
        KEY_AUDIOPREV = 259,
        KEY_AUDIOSTOP = 260,
        KEY_AUDIOPLAY = 261,
        KEY_AUDIOMUTE = 262,
        KEY_MEDIASELECT = 263,
        KEY_WWW = 264,
        KEY_MAIL = 265,
        KEY_CALCULATOR = 266,
        KEY_COMPUTER = 267,
        KEY_AC_SEARCH = 268,
        KEY_AC_HOME = 269,
        KEY_AC_BACK = 270,
        KEY_AC_FORWARD = 271,
        KEY_AC_STOP = 272,
        KEY_AC_REFRESH = 273,
        KEY_AC_BOOKMARKS = 274,

        KEY_BRIGHTNESSDOWN = 275,
        KEY_BRIGHTNESSUP = 276,
        KEY_DISPLAYSWITCH = 277,

        KEY_KBDILLUMTOGGLE = 278,
        KEY_KBDILLUMDOWN = 279,
        KEY_KBDILLUMUP = 280,
        KEY_EJECT = 281,
        KEY_SLEEP = 282,

        KEY_APP1 = 283,
        KEY_APP2 = 284,

        KEY_AUDIOREWIND = 285,
        KEY_AUDIOFASTFORWARD = 286,

        KEY_SOFTLEFT = 287,

        KEY_SOFTRIGHT = 288,

        KEY_CALL = 289,
        KEY_ENDCALL = 290,
        NUM_SCANCODES = 512
    };
}