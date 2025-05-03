using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class TestScript_2 : MonoBehaviour
{
    private IGameObject game_object;
	
    public string prefab_path = "Assets/Prefabs/Samurai_Prefab_Test.prefab";
    public string scene_path = "Assets/Scenes/Pong_Scene_test.mlscene";

    public override void Start()
    {
        game_object = attached_game_object;
        InternalCalls.ScriptingLog("Start TestScript_2", __arglist());
    }

    public override void Update()
    {
        if (InternalCalls.GetKeyboardKeyDown((int)KeyboardKey.SDL_SCANCODE_Q))
        {
            InternalCalls.InstantiatePrefab(prefab_path);
        }
        if (InternalCalls.GetKeyboardKeyDown((int)KeyboardKey.SDL_SCANCODE_E))
        {
            InternalCalls.LoadScene(scene_path);
        }
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