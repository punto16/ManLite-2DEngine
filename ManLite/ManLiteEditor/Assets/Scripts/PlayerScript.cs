using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class PlayerScript : MonoBehaviour
{
    private bool jump_available = false;

    private Collider2D collider;

    private Light light;

    public override void Start()
    {
        collider = new Collider2D(attached_game_object);
        light = new Light(attached_game_object);

        ML_System.Log("Start PlayerScript");
        ML_System.Log("Log Test | Int: {0}", 69);
        ML_System.Log("Log Test | Float: {0}", 69.420f);
        ML_System.Log("Log Test | String: {0}", "Kilo Niga");
        ML_System.Log("Log Test | Bool: {0}", true);
    }

    public override void Update()
    {
        
        Vec2f speed = collider.GetSpeed();

        if (Input.GetKeyboardKey(KeyboardKey.SDL_SCANCODE_A) == KeyState.KEY_REPEAT)
        {
            ML_System.Log("PlayerScript Left");

            speed.X = -10;
        }
        if (Input.GetKeyboardKey(KeyboardKey.SDL_SCANCODE_D) == KeyState.KEY_REPEAT)
        {
            ML_System.Log("PlayerScript Right");

            speed.X = 10;
        }

        if (jump_available && Input.GetKeyboardKey(KeyboardKey.SDL_SCANCODE_SPACE) == KeyState.KEY_DOWN)
        {
            ML_System.Log("PlayerScript Jump");

            speed.Y = 10;

            jump_available = false;
        }

        if (Input.GetKeyboardKey(KeyboardKey.SDL_SCANCODE_W) == KeyState.KEY_REPEAT)
        {
            float intensity = light.GetIntensity();

            intensity += 0.01f;

            light.SetIntensity(intensity);
        }
        if (Input.GetKeyboardKey(KeyboardKey.SDL_SCANCODE_S) == KeyState.KEY_REPEAT)
        {
            float intensity = light.GetIntensity();

            intensity -= 0.01f;

            light.SetIntensity(intensity);
        }

        collider.SetSpeed(speed);
    }

    public override void OnTriggerCollision(IGameObject other)
    {
        ML_System.Log("Collision IN");
    }
    public override void OnTriggerSensor(IGameObject other)
    {
        ML_System.Log("Sensor IN");

        jump_available = true;
    }
    public override void OnExitCollision(IGameObject other)
    {
        ML_System.Log("Collision OUT");
    }
    public override void OnExitSensor(IGameObject other)
    {
        ML_System.Log("Sensor OUT");

        jump_available = false;
    }
}