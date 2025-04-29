using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class TestScript_1 : MonoBehaviour
{
    private IGameObject game_object;
	
    public int players_speed = 10;
    public float ball_speed = 0.420f;
    public bool AllowUpdate = true;
    public IGameObject player_1;
    public IGameObject player_2;
    public IGameObject ball;
    public string string_example = "test";

    public override void Start()
    {
        game_object = attached_game_object;
        InternalCalls.ScriptingLog("Start TestScript_1", __arglist());

        InternalCalls.Vec2f forceball;
        forceball.X = ball_speed;
        forceball.Y = ball_speed;
        InternalCalls.SetSpeedCollider(ball.game_object_ptr, forceball);
    }

    public override void Update()
    {
        float dt = InternalCalls.GetDT();
        InternalCalls.Vec2f force;
        force.X = 0;
        force.Y = 0;
        InternalCalls.Vec2f force2;
        force2.X = 0;
        force2.Y = 0;
        if (InternalCalls.GetKeyboardKeyRepeat((int)KeyboardKey.SDL_SCANCODE_W))
        {
            force.Y += players_speed;
        }
        if (InternalCalls.GetKeyboardKeyRepeat((int)KeyboardKey.SDL_SCANCODE_S))
        {
            force.Y = -players_speed;
        }
        if (InternalCalls.GetKeyboardKeyRepeat((int)KeyboardKey.SDL_SCANCODE_UP))
        {
            force2.Y += players_speed;
        }
        if (InternalCalls.GetKeyboardKeyRepeat((int)KeyboardKey.SDL_SCANCODE_DOWN))
        {
            force2.Y = -players_speed;
        }

        if (InternalCalls.GetKeyboardKeyRepeat((int)KeyboardKey.SDL_SCANCODE_P))
        {
            InternalCalls.LoadScene("Assets/Scenes/Pong_Scene_test.mlscene");
        }

        if (!AllowUpdate)
        {
            InternalCalls.Vec2f forceball;
            forceball.X = ball_speed;
            forceball.Y = ball_speed;
            InternalCalls.SetSpeedCollider(ball.game_object_ptr, forceball);
        }

        InternalCalls.SetSpeedCollider(player_1.game_object_ptr, force);
        InternalCalls.SetSpeedCollider(player_2.game_object_ptr, force2);
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