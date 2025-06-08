using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class SamuraiMovement : MonoBehaviour
{
    enum AnimationSamuraiState
    {
        IDLE,
        RUN_LEFT,
        RUN_RIGHT
    }

    public IGameObject samurai_animations;

    //components
    private Transform transform;
    private Collider2D collider;
    private Animator animator;
    private Audio player_audios;

    private AnimationSamuraiState animation_state = AnimationSamuraiState.IDLE;

    public override void Start()
    {
        transform = new Transform(attached_game_object);
        collider = new Collider2D(attached_game_object);
        animator = new Animator(samurai_animations);
        player_audios = new Audio(attached_game_object);
    }

    public override void Update()
    {
        Vec2f speed = collider.GetSpeed();

        //movement
        if (Input.GetKeyboardKey(KeyboardKey.KEY_A) == KeyState.KEY_REPEAT)
        {
            speed.X = -4.0f;
        }
        if (Input.GetKeyboardKey(KeyboardKey.KEY_D) == KeyState.KEY_REPEAT)
        {
            speed.X = 4.0f;
        }
        if (Input.GetKeyboardKey(KeyboardKey.KEY_R) == KeyState.KEY_DOWN)
        {
            Vec2f newpos = new Vec2f(2.0f, -3.8f);
            transform.SetWorldPosition(newpos);
        }
        if (Input.GetKeyboardKey(KeyboardKey.KEY_1) == KeyState.KEY_DOWN)
        {
            player_audios.PauseAll();
        }
        if (Input.GetKeyboardKey(KeyboardKey.KEY_2) == KeyState.KEY_DOWN)
        {
            player_audios.UnpauseAll();
        }
        if (Input.GetKeyboardKey(KeyboardKey.KEY_3) == KeyState.KEY_DOWN)
        {
            player_audios.StopAll();
        }

        //animation and sound
        if (animation_state != AnimationSamuraiState.RUN_LEFT &&
            Input.GetKeyboardKey(KeyboardKey.KEY_A) == KeyState.KEY_DOWN)
        {
            animator.PlayAnimation("samurai_run_left");
            animation_state = AnimationSamuraiState.RUN_LEFT;
        }
        if (animation_state != AnimationSamuraiState.RUN_RIGHT &&
            Input.GetKeyboardKey(KeyboardKey.KEY_D) == KeyState.KEY_DOWN)
        {
            animator.PlayAnimation("samurai_run_right");
            animation_state = AnimationSamuraiState.RUN_RIGHT;
        }
        if (animation_state != AnimationSamuraiState.IDLE &&
            Input.GetKeyboardKey(KeyboardKey.KEY_D) == KeyState.KEY_IDLE &&
            Input.GetKeyboardKey(KeyboardKey.KEY_A) == KeyState.KEY_IDLE)
        {
            animator.PlayAnimation("samurai_idle");
            animation_state = AnimationSamuraiState.IDLE;
            player_audios.StopSound("walk_on_grass_sound_effect");
        }
        if (animation_state == AnimationSamuraiState.RUN_LEFT ||
            animation_state == AnimationSamuraiState.RUN_RIGHT)
        {
            player_audios.PlaySound("walk_on_grass_sound_effect");
        }
        
        collider.SetSpeed(speed);
    }

    public override void OnTriggerCollision(IGameObject other)
    {

    }
    public override void OnTriggerSensor(IGameObject other)
    {

    }
    public override void OnExitCollision(IGameObject other)
    {

    }
    public override void OnExitSensor(IGameObject other)
    {

    }
}