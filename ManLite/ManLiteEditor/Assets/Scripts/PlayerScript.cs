using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class PlayerScript : MonoBehaviour
{
    private bool jump_available = false;
    private bool orb_jump_available = false;

    private bool alive = true;
    private float death_time = 2.0f; //2 seconds
    private float death_timer = 0.0f;

    private Vec2f spawn_pos;

    //components

    private Transform transform;
    private Collider2D collider;
    private Particles friction_floor_particles;
    private Audio level_music;

    public override void Start()
    {
        spawn_pos = new Vec2f();
        spawn_pos.X = 0;
        spawn_pos.Y = 0;

        transform = new Transform(attached_game_object);
        collider = new Collider2D(attached_game_object);
        friction_floor_particles = new Particles(attached_game_object);
        level_music = new Audio(attached_game_object);
    }

    public override void Update()
    {
        Vec2f speed = collider.GetSpeed();
        if (!alive)
        {
            speed.X = 0;
            speed.Y = 0;
            collider.SetSpeed(speed);

            death_timer = death_timer + ML_System.GetDT();
            if (death_timer >= death_time)
            {
                death_timer = 0.0f;
                alive = true;
                level_music.PlayMusic("Polargeist");
                transform.SetWorldPosition(spawn_pos);
            }
            return;
        }
        else
        {
            death_timer = 0;
        }

        speed.X = 7;

        if (orb_jump_available)
        {
            if (Input.GetKeyboardKey(KeyboardKey.KEY_SPACE) == KeyState.KEY_DOWN)
            {
                speed.Y = 11.2f;
            }
            if (Input.GetMouseButton(MouseButton.MOUSE_BUTTON_LEFT) == KeyState.KEY_DOWN)
            {
                speed.Y = 11.2f;
            }
        }
        else if (jump_available)
        {
            if (Input.GetKeyboardKey(KeyboardKey.KEY_SPACE) == KeyState.KEY_REPEAT)
            {
                speed.Y = 11.2f;
            }
            if (Input.GetMouseButton(MouseButton.MOUSE_BUTTON_LEFT) == KeyState.KEY_REPEAT)
            {
                speed.Y = 11.2f;
            }
        }

        collider.SetSpeed(speed);

    }

    public override void OnTriggerCollision(IGameObject other)
    {
        if (other.tag == "block")
        {
            jump_available = true;
            friction_floor_particles.Play();
	        ML_System.Log("Collision with block now");
        }
    }
    public override void OnTriggerSensor(IGameObject other)
    {
        if (other.tag == "Kill" || other.tag == "block_left")
        {
            //die
            alive = false;
            level_music.StopMusic("Polargeist");
	        ML_System.Log("Player Killed");
        }
        if (other.tag == "yellow_orb")
        {
            orb_jump_available = true;
        }
        if (other.tag == "yellow_pad")
        {
            Vec2f speed = collider.GetSpeed();
            speed.Y = 15;
            collider.SetSpeed(speed);
        }
    }
    public override void OnExitCollision(IGameObject other)
    {
        if (other.tag == "block")
        {
            jump_available = false;
            friction_floor_particles.Stop();
	        ML_System.Log("END Collision with block now");
        }
    }
    public override void OnExitSensor(IGameObject other)
    {
        if (other.tag == "yellow_orb")
        {
            orb_jump_available = false;
        }
    }
}