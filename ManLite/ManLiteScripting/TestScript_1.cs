using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class TestScript_1 : MonoBehaviour
{
    IGameObject game_object;

    public override void Start()
    {
        game_object = attached_game_object;
        InternalCalls.ScriptingLog("Start TestScript_1", __arglist());
    }

    public override void Update()
    {
        float dt = InternalCalls.GetDT();

        if (InternalCalls.GetKeyboardKeyDown((int)KeyboardKey.SDL_SCANCODE_W))
        {
            InternalCalls.Vec2f force;
            force.X = 0;
            force.Y = 10;
            InternalCalls.SetSpeedCollider(attached_game_object.game_object_ptr, force);
            InternalCalls.ScriptingLog("Key W Pressed from script!", __arglist());
        }
        if (InternalCalls.GetKeyboardKeyDown((int)KeyboardKey.SDL_SCANCODE_S))
        {
            InternalCalls.Vec2f force;
            force.X = 0;
            force.Y = -10;
            InternalCalls.SetSpeedCollider(attached_game_object.game_object_ptr, force);
            InternalCalls.ScriptingLog("Key S Pressed from script!", __arglist());
        }
    }
}