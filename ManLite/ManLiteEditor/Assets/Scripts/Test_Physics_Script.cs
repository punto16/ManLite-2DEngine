
using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class Test_Physics_Script : MonoBehaviour
{
    //create variables here

    private float players_speed = 10.0f;

    private Collider2D collider;

    //will be called once
    public override void Start()
    {
        collider = new Collider2D(attached_game_object);
    }

    //it is called every frame
    public override void Update()
    {
        Vec2f seed = collider.GetSpeed();
        seed.X = 0.0f;

        if (Input.GetKeyboardKey(KeyboardKey.KEY_A) == KeyState.KEY_REPEAT)
        {
            seed.X -= players_speed;
        }
        if (Input.GetKeyboardKey(KeyboardKey.KEY_D) == KeyState.KEY_REPEAT)
        {
            seed.X += players_speed;
        }
        if (Input.GetKeyboardKey(KeyboardKey.KEY_SPACE) == KeyState.KEY_DOWN)
        {
            seed.Y += players_speed;
        }

        collider.SetSpeed(seed);
    }

    //functions for colliders
    public override void OnTriggerCollision(IGameObject other)
    {
        ML_System.Log("Collision with block now");
    }
    public override void OnExitCollision(IGameObject other)
    {
        ML_System.Log("END Collision with block now");
    }

    public override void OnTriggerSensor(IGameObject other)
    {
    }
    public override void OnExitSensor(IGameObject other)
    {
    }
}