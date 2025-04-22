using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class TestScript_1 : MonoBehaviour
{
    private IGameObject game_object;
	
    public int ForceX = 10;
    public int ForceY = 10;
    public float float_example = 0.420f;
    public bool AllowUpdate = true;
    //public IGameObject go_example;
    public string string_example = "test";

    public override void Start()
    {
        game_object = attached_game_object;
        InternalCalls.ScriptingLog("Start TestScript_1", __arglist());
    }

    public override void Update()
    {
        float dt = InternalCalls.GetDT();
        if (!AllowUpdate) return;
        InternalCalls.Vec2f force;
        force.X = 0;
        force.Y = 0;
        if (InternalCalls.GetKeyboardKeyRepeat((int)KeyboardKey.SDL_SCANCODE_W))
        {
            force.Y += ForceY;
        }
        if (InternalCalls.GetKeyboardKeyRepeat((int)KeyboardKey.SDL_SCANCODE_S))
        {
            force.Y = -ForceY;
        }
        if (InternalCalls.GetKeyboardKeyRepeat((int)KeyboardKey.SDL_SCANCODE_D))
        {
            force.X += ForceX;
        }
        if (InternalCalls.GetKeyboardKeyRepeat((int)KeyboardKey.SDL_SCANCODE_A))
        {
            force.X = -ForceX;
        }

        InternalCalls.SetSpeedCollider(attached_game_object.game_object_ptr, force);
    }

    public override void OnTriggerCollision(IGameObject other)
    {
        InternalCalls.ScriptingLog("Collision IN", __arglist());
    }
    public override void OnTriggerSensor(IGameObject other)
    {
        InternalCalls.ScriptingLog("Sensor IN", __arglist());
    }
    public override void OnExitCollision(IGameObject other)
    {
        InternalCalls.ScriptingLog("Collision OUT", __arglist());
    }
    public override void OnExitSensor(IGameObject other)
    {
        InternalCalls.ScriptingLog("Sensor OUT", __arglist());
    }
}