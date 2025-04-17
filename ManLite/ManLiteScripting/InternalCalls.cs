using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace ManLiteScripting
{
    public class InternalCalls
    {
        #region Structs
        [StructLayout(LayoutKind.Sequential)]
        public struct Vec2f
        {
            public float X;
            public float Y;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct Vec4f
        {
            public float X;
            public float Y;
            public float Z;
            public float W;
        }
        #endregion


        #region GameObject
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static IntPtr GetGOPtr();
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static string GetGOName(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetGOName(IntPtr go, string name);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static string GetGOTag(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetGOTag(IntPtr go, string tag);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static uint GetGOID(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static IntPtr GetGOParent(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetGOEnabled(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetGOEnabled(IntPtr go, bool enable);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SwitchGOEnabled(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetGOVisible(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetGOVisible(IntPtr go, bool visible);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SwitchGOVisible(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void ReparentGO(IntPtr go, IntPtr newParent);
        #endregion


        #region Input
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetKeyboardKeyDown(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetKeyboardKeyRepeat(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetKeyboardKeyUp(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetKeyboardKeyIdle(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetMouseButtonDown(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetMouseButtonRepeat(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetMouseButtonUp(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetMouseButtonIdle(int id);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetMousePosition();
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetMouseMotion();
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static int GetMouseWheelMotion();
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void CloseApp();
        #endregion


        #region Scene Management
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void LoadScene(string path);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void CreateEmptyGO(IntPtr parent);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void DeleteGO(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static IntPtr FindGameObjectByID(uint id);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static IntPtr FindGameObjectByName(string name);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static IntPtr GetSceneRoot();
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static IntPtr GetCurrentCameraGO();
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetCurrentCameraGO(IntPtr cameraGO);
        #endregion


        #region Utils
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static float GetDT();
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void ScriptingLog(string format, __arglist);
        #endregion


        #region Components
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool IsComponentEnabled(IntPtr go, int componentType);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetEnableComponent(IntPtr go, int componentType, bool enable);
        #endregion


        #region Transform
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetLocalPosition(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetLocalPosition(IntPtr go, Vec2f pos);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static float GetLocalAngle(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetLocalAngle(IntPtr go, float angle);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetLocalScale(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetLocalScale(IntPtr go, Vec2f scale);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetWorldPosition(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetWorldPosition(IntPtr go, Vec2f pos);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static float GetWorldAngle(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetWorldAngle(IntPtr go, float angle);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetWorldScale(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetWorldScale(IntPtr go, Vec2f scale);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool IsAspectRatioLocked(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetAspectRatioLock(IntPtr go, bool enable);
        #endregion


        #region Camera
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetViewportSize(IntPtr cameraGO);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetViewportSize(IntPtr cameraGO, Vec2f size);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static float GetZoom(IntPtr cameraGO);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetZoom(IntPtr cameraGO, float zoom);
        #endregion


        #region Sprite
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetTextureSize(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec4f GetTextureSection(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetTextureSection(IntPtr go, Vec4f section);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetOffset(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetOffset(IntPtr go, Vec2f offset);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool IsPixelArt(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetPixelArtRender(IntPtr go, bool enable);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SwapTexture(IntPtr go, string path);
        #endregion


        #region Animator
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void PlayAnimation(IntPtr go, string animation);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void StopAnimation(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool IsAnimaitonPlaying(IntPtr go, string animation);
        #endregion


        #region Collider2D
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void PauseCollider2D(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void UnpauseCollider2D(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static int GetShapeType(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetShapeType(IntPtr go, int shapeType);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetColliderSize(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetColliderSize(IntPtr go, Vec2f size);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static float GetColliderRadius(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetColliderRadius(IntPtr go, float radius);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool IsCollierDynamic(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetColliderDynamic(IntPtr go, bool dynamic);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool IsCollierSensor(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetColliderSensor(IntPtr go, bool sensor);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool IsCollierRotationLocked(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetCollierRotationLocked(IntPtr go, bool locked);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool IsCollierUsingGravity(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetCollierUsingGravity(IntPtr go, bool useGravity);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void ApplyForceCollider(IntPtr go, Vec2f force);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetSpeedCollider(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetSpeedCollider(IntPtr go, Vec2f speed);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static float GetColliderFriction(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetColliderFriction(IntPtr go, float friction);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static float GetColliderLinearDamping(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetColliderLinearDamping(IntPtr go, float damping);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static float GetColliderMass(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetColliderMass(IntPtr go, float mass);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static float GetColliderRestitution(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetColliderRestitution(IntPtr go, float restitution);
        #endregion


        #region Canvas
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static int GetStateButtonUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetStateButtonUI(IntPtr go, string itemUI, int state);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static int GetStateCheckboxUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetStateCheckboxUI(IntPtr go, string itemUI, int state);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetValueCheckboxUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetValueCheckboxUI(IntPtr go, string itemUI, bool value);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static int GetStateSliderUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetStateSliderUI(IntPtr go, string itemUI, int state);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static int GetValueSliderUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetValueSliderUI(IntPtr go, string itemUI, int value);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static string GetTextTextUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetTextTextUI(IntPtr go, string itemUI, string text);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetItemUIPosition(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetItemUIPosition(IntPtr go, string itemUI, Vec2f position);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static float GetItemUIAngle(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetItemUIAngle(IntPtr go, string itemUI, float angle);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static Vec2f GetItemUIScale(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetItemUIScale(IntPtr go, string itemUI, Vec2f scale);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool GetItemUIAspectLock(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetItemUIAspectLock(IntPtr go, string itemUI, bool locked);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static bool IsEnabledItemUI(IntPtr go, string itemUI);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void EnableItemUI(IntPtr go, string itemUI, bool enabled);
        #endregion


        #region Audio
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void PauseAudioSource(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void UnpauseAudioSource(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void StopAudioSource(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void PlayMusic(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void PlaySound(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void PauseMusic(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void PauseSound(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void StopMusic(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void StopSound(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static int GetMusicVolume(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static int GetSoundVolume(IntPtr go, string audio);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetMusicVolume(IntPtr go, string audio, int volume);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void SetSoundVolume(IntPtr go, string audio, int volume);
        #endregion


        #region Particles
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void PlayParticleSystem(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void PauseParticleSystem(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void UnpauseParticleSystem(IntPtr go);
        [MethodImpl(MethodImplOptions.InternalCall)] internal extern static void StopParticleSystem(IntPtr go);
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


        Unkown
    };

    public enum MouseButton
    {
        SDL_BUTTON_UNKNOWN = 0,
        SDL_BUTTON_LEFT = 1,
        SDL_BUTTON_MIDDLE = 2,
        SDL_BUTTON_RIGHT = 3,
        SDL_BUTTON_X1 = 4,
        SDL_BUTTON_X2 = 5
    };

    public enum KeyboardKey
    {
        SDL_SCANCODE_UNKNOWN = 0,
        SDL_SCANCODE_A = 4,
        SDL_SCANCODE_B = 5,
        SDL_SCANCODE_C = 6,
        SDL_SCANCODE_D = 7,
        SDL_SCANCODE_E = 8,
        SDL_SCANCODE_F = 9,
        SDL_SCANCODE_G = 10,
        SDL_SCANCODE_H = 11,
        SDL_SCANCODE_I = 12,
        SDL_SCANCODE_J = 13,
        SDL_SCANCODE_K = 14,
        SDL_SCANCODE_L = 15,
        SDL_SCANCODE_M = 16,
        SDL_SCANCODE_N = 17,
        SDL_SCANCODE_O = 18,
        SDL_SCANCODE_P = 19,
        SDL_SCANCODE_Q = 20,
        SDL_SCANCODE_R = 21,
        SDL_SCANCODE_S = 22,
        SDL_SCANCODE_T = 23,
        SDL_SCANCODE_U = 24,
        SDL_SCANCODE_V = 25,
        SDL_SCANCODE_W = 26,
        SDL_SCANCODE_X = 27,
        SDL_SCANCODE_Y = 28,
        SDL_SCANCODE_Z = 29,

        SDL_SCANCODE_1 = 30,
        SDL_SCANCODE_2 = 31,
        SDL_SCANCODE_3 = 32,
        SDL_SCANCODE_4 = 33,
        SDL_SCANCODE_5 = 34,
        SDL_SCANCODE_6 = 35,
        SDL_SCANCODE_7 = 36,
        SDL_SCANCODE_8 = 37,
        SDL_SCANCODE_9 = 38,
        SDL_SCANCODE_0 = 39,

        SDL_SCANCODE_RETURN = 40,
        SDL_SCANCODE_ESCAPE = 41,
        SDL_SCANCODE_BACKSPACE = 42,
        SDL_SCANCODE_TAB = 43,
        SDL_SCANCODE_SPACE = 44,

        SDL_SCANCODE_MINUS = 45,
        SDL_SCANCODE_EQUALS = 46,
        SDL_SCANCODE_LEFTBRACKET = 47,
        SDL_SCANCODE_RIGHTBRACKET = 48,
        SDL_SCANCODE_BACKSLASH = 49,
        SDL_SCANCODE_NONUSHASH = 50,
        SDL_SCANCODE_SEMICOLON = 51,
        SDL_SCANCODE_APOSTROPHE = 52,
        SDL_SCANCODE_GRAVE = 53,
        SDL_SCANCODE_COMMA = 54,
        SDL_SCANCODE_PERIOD = 55,
        SDL_SCANCODE_SLASH = 56,

        SDL_SCANCODE_CAPSLOCK = 57,

        SDL_SCANCODE_F1 = 58,
        SDL_SCANCODE_F2 = 59,
        SDL_SCANCODE_F3 = 60,
        SDL_SCANCODE_F4 = 61,
        SDL_SCANCODE_F5 = 62,
        SDL_SCANCODE_F6 = 63,
        SDL_SCANCODE_F7 = 64,
        SDL_SCANCODE_F8 = 65,
        SDL_SCANCODE_F9 = 66,
        SDL_SCANCODE_F10 = 67,
        SDL_SCANCODE_F11 = 68,
        SDL_SCANCODE_F12 = 69,

        SDL_SCANCODE_PRINTSCREEN = 70,
        SDL_SCANCODE_SCROLLLOCK = 71,
        SDL_SCANCODE_PAUSE = 72,
        SDL_SCANCODE_INSERT = 73,
        SDL_SCANCODE_HOME = 74,
        SDL_SCANCODE_PAGEUP = 75,
        SDL_SCANCODE_DELETE = 76,
        SDL_SCANCODE_END = 77,
        SDL_SCANCODE_PAGEDOWN = 78,
        SDL_SCANCODE_RIGHT = 79,
        SDL_SCANCODE_LEFT = 80,
        SDL_SCANCODE_DOWN = 81,
        SDL_SCANCODE_UP = 82,

        SDL_SCANCODE_NUMLOCKCLEAR = 83,
        SDL_SCANCODE_KP_DIVIDE = 84,
        SDL_SCANCODE_KP_MULTIPLY = 85,
        SDL_SCANCODE_KP_MINUS = 86,
        SDL_SCANCODE_KP_PLUS = 87,
        SDL_SCANCODE_KP_ENTER = 88,
        SDL_SCANCODE_KP_1 = 89,
        SDL_SCANCODE_KP_2 = 90,
        SDL_SCANCODE_KP_3 = 91,
        SDL_SCANCODE_KP_4 = 92,
        SDL_SCANCODE_KP_5 = 93,
        SDL_SCANCODE_KP_6 = 94,
        SDL_SCANCODE_KP_7 = 95,
        SDL_SCANCODE_KP_8 = 96,
        SDL_SCANCODE_KP_9 = 97,
        SDL_SCANCODE_KP_0 = 98,
        SDL_SCANCODE_KP_PERIOD = 99,

        SDL_SCANCODE_NONUSBACKSLASH = 100,
        SDL_SCANCODE_APPLICATION = 101,
        SDL_SCANCODE_POWER = 102,

        SDL_SCANCODE_KP_EQUALS = 103,
        SDL_SCANCODE_F13 = 104,
        SDL_SCANCODE_F14 = 105,
        SDL_SCANCODE_F15 = 106,
        SDL_SCANCODE_F16 = 107,
        SDL_SCANCODE_F17 = 108,
        SDL_SCANCODE_F18 = 109,
        SDL_SCANCODE_F19 = 110,
        SDL_SCANCODE_F20 = 111,
        SDL_SCANCODE_F21 = 112,
        SDL_SCANCODE_F22 = 113,
        SDL_SCANCODE_F23 = 114,
        SDL_SCANCODE_F24 = 115,
        SDL_SCANCODE_EXECUTE = 116,
        SDL_SCANCODE_HELP = 117,
        SDL_SCANCODE_MENU = 118,
        SDL_SCANCODE_SELECT = 119,
        SDL_SCANCODE_STOP = 120,
        SDL_SCANCODE_AGAIN = 121,
        SDL_SCANCODE_UNDO = 122,
        SDL_SCANCODE_CUT = 123,
        SDL_SCANCODE_COPY = 124,
        SDL_SCANCODE_PASTE = 125,
        SDL_SCANCODE_FIND = 126,
        SDL_SCANCODE_MUTE = 127,
        SDL_SCANCODE_VOLUMEUP = 128,
        SDL_SCANCODE_VOLUMEDOWN = 129,

        SDL_SCANCODE_KP_COMMA = 133,
        SDL_SCANCODE_KP_EQUALSAS400 = 134,

        SDL_SCANCODE_INTERNATIONAL1 = 135,

        SDL_SCANCODE_INTERNATIONAL2 = 136,
        SDL_SCANCODE_INTERNATIONAL3 = 137,
        SDL_SCANCODE_INTERNATIONAL4 = 138,
        SDL_SCANCODE_INTERNATIONAL5 = 139,
        SDL_SCANCODE_INTERNATIONAL6 = 140,
        SDL_SCANCODE_INTERNATIONAL7 = 141,
        SDL_SCANCODE_INTERNATIONAL8 = 142,
        SDL_SCANCODE_INTERNATIONAL9 = 143,
        SDL_SCANCODE_LANG1 = 144,
        SDL_SCANCODE_LANG2 = 145,
        SDL_SCANCODE_LANG3 = 146,
        SDL_SCANCODE_LANG4 = 147,
        SDL_SCANCODE_LANG5 = 148,
        SDL_SCANCODE_LANG6 = 149,
        SDL_SCANCODE_LANG7 = 150,
        SDL_SCANCODE_LANG8 = 151,
        SDL_SCANCODE_LANG9 = 152,

        SDL_SCANCODE_ALTERASE = 153,
        SDL_SCANCODE_SYSREQ = 154,
        SDL_SCANCODE_CANCEL = 155,
        SDL_SCANCODE_CLEAR = 156,
        SDL_SCANCODE_PRIOR = 157,
        SDL_SCANCODE_RETURN2 = 158,
        SDL_SCANCODE_SEPARATOR = 159,
        SDL_SCANCODE_OUT = 160,
        SDL_SCANCODE_OPER = 161,
        SDL_SCANCODE_CLEARAGAIN = 162,
        SDL_SCANCODE_CRSEL = 163,
        SDL_SCANCODE_EXSEL = 164,

        SDL_SCANCODE_KP_00 = 176,
        SDL_SCANCODE_KP_000 = 177,
        SDL_SCANCODE_THOUSANDSSEPARATOR = 178,
        SDL_SCANCODE_DECIMALSEPARATOR = 179,
        SDL_SCANCODE_CURRENCYUNIT = 180,
        SDL_SCANCODE_CURRENCYSUBUNIT = 181,
        SDL_SCANCODE_KP_LEFTPAREN = 182,
        SDL_SCANCODE_KP_RIGHTPAREN = 183,
        SDL_SCANCODE_KP_LEFTBRACE = 184,
        SDL_SCANCODE_KP_RIGHTBRACE = 185,
        SDL_SCANCODE_KP_TAB = 186,
        SDL_SCANCODE_KP_BACKSPACE = 187,
        SDL_SCANCODE_KP_A = 188,
        SDL_SCANCODE_KP_B = 189,
        SDL_SCANCODE_KP_C = 190,
        SDL_SCANCODE_KP_D = 191,
        SDL_SCANCODE_KP_E = 192,
        SDL_SCANCODE_KP_F = 193,
        SDL_SCANCODE_KP_XOR = 194,
        SDL_SCANCODE_KP_POWER = 195,
        SDL_SCANCODE_KP_PERCENT = 196,
        SDL_SCANCODE_KP_LESS = 197,
        SDL_SCANCODE_KP_GREATER = 198,
        SDL_SCANCODE_KP_AMPERSAND = 199,
        SDL_SCANCODE_KP_DBLAMPERSAND = 200,
        SDL_SCANCODE_KP_VERTICALBAR = 201,
        SDL_SCANCODE_KP_DBLVERTICALBAR = 202,
        SDL_SCANCODE_KP_COLON = 203,
        SDL_SCANCODE_KP_HASH = 204,
        SDL_SCANCODE_KP_SPACE = 205,
        SDL_SCANCODE_KP_AT = 206,
        SDL_SCANCODE_KP_EXCLAM = 207,
        SDL_SCANCODE_KP_MEMSTORE = 208,
        SDL_SCANCODE_KP_MEMRECALL = 209,
        SDL_SCANCODE_KP_MEMCLEAR = 210,
        SDL_SCANCODE_KP_MEMADD = 211,
        SDL_SCANCODE_KP_MEMSUBTRACT = 212,
        SDL_SCANCODE_KP_MEMMULTIPLY = 213,
        SDL_SCANCODE_KP_MEMDIVIDE = 214,
        SDL_SCANCODE_KP_PLUSMINUS = 215,
        SDL_SCANCODE_KP_CLEAR = 216,
        SDL_SCANCODE_KP_CLEARENTRY = 217,
        SDL_SCANCODE_KP_BINARY = 218,
        SDL_SCANCODE_KP_OCTAL = 219,
        SDL_SCANCODE_KP_DECIMAL = 220,
        SDL_SCANCODE_KP_HEXADECIMAL = 221,

        SDL_SCANCODE_LCTRL = 224,
        SDL_SCANCODE_LSHIFT = 225,
        SDL_SCANCODE_LALT = 226,
        SDL_SCANCODE_LGUI = 227,
        SDL_SCANCODE_RCTRL = 228,
        SDL_SCANCODE_RSHIFT = 229,
        SDL_SCANCODE_RALT = 230,
        SDL_SCANCODE_RGUI = 231,

        SDL_SCANCODE_MODE = 257,

        SDL_SCANCODE_AUDIONEXT = 258,
        SDL_SCANCODE_AUDIOPREV = 259,
        SDL_SCANCODE_AUDIOSTOP = 260,
        SDL_SCANCODE_AUDIOPLAY = 261,
        SDL_SCANCODE_AUDIOMUTE = 262,
        SDL_SCANCODE_MEDIASELECT = 263,
        SDL_SCANCODE_WWW = 264,
        SDL_SCANCODE_MAIL = 265,
        SDL_SCANCODE_CALCULATOR = 266,
        SDL_SCANCODE_COMPUTER = 267,
        SDL_SCANCODE_AC_SEARCH = 268,
        SDL_SCANCODE_AC_HOME = 269,
        SDL_SCANCODE_AC_BACK = 270,
        SDL_SCANCODE_AC_FORWARD = 271,
        SDL_SCANCODE_AC_STOP = 272,
        SDL_SCANCODE_AC_REFRESH = 273,
        SDL_SCANCODE_AC_BOOKMARKS = 274,

        SDL_SCANCODE_BRIGHTNESSDOWN = 275,
        SDL_SCANCODE_BRIGHTNESSUP = 276,
        SDL_SCANCODE_DISPLAYSWITCH = 277,

        SDL_SCANCODE_KBDILLUMTOGGLE = 278,
        SDL_SCANCODE_KBDILLUMDOWN = 279,
        SDL_SCANCODE_KBDILLUMUP = 280,
        SDL_SCANCODE_EJECT = 281,
        SDL_SCANCODE_SLEEP = 282,

        SDL_SCANCODE_APP1 = 283,
        SDL_SCANCODE_APP2 = 284,

        SDL_SCANCODE_AUDIOREWIND = 285,
        SDL_SCANCODE_AUDIOFASTFORWARD = 286,

        SDL_SCANCODE_SOFTLEFT = 287,

        SDL_SCANCODE_SOFTRIGHT = 288,

        SDL_SCANCODE_CALL = 289,
        SDL_SCANCODE_ENDCALL = 290,
        SDL_NUM_SCANCODES = 512
    };
}
