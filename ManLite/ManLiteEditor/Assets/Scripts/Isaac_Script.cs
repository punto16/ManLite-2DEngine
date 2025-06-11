
using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public enum Direction
{
    TOP,
    DOWN,
    LEFT,
    RIGHT
}

public class Isaac_Script : MonoBehaviour
{
    //create variables here
    public Direction head_direction = Direction.DOWN;
    public Direction body_direction = Direction.DOWN;

    public float player_speed = 5.0f;
    public float tear_speed = 7.0f;
    public float fire_rate = 1.0f;
    private float fire_rate_timer = 0.0f;

    public IGameObject head_go;
    public IGameObject body_go;
    public IGameObject shadow_go;
    public IGameObject canvas_go;

    private Animator head_animator;
    private Animator body_animator;

    private Collider2D collider;
    private Transform transform;
    private Audio audio;
    private Particles particles;

    private bool shooting = false;
    private bool walking = false;

    private int original_hp = 3;
    public int hp = 3;

    private float unhittable_time = 1.0f;
    private float unhittable_timer = 0.0f;
    private bool unhittable = false;

    //will be called once
    public override void Start()
    {
        Canvas canvas = new Canvas(canvas_go);
        canvas.SetText("health_ui", hp.ToString());

        head_animator = new Animator(head_go);
        body_animator = new Animator(body_go);
        collider = new Collider2D(attached_game_object);
        transform = new Transform(attached_game_object);
        audio = new Audio(attached_game_object);
        particles = new Particles(attached_game_object);
    }

    //it is called every frame
    public override void Update()
    {
        float dt = ML_System.GetDT();

        if (unhittable)
        {
            unhittable_timer += dt;
            if (unhittable_timer >= unhittable_time)
            {
                unhittable = false;
                unhittable_timer = 0.0f;
                if (hp <= 0) Scene.SetBackGroundLoadedScene();
            }
        }
        if (hp <= 0) return;

        //handle input
        walking = true;
        if      (Input.GetKeyboardKey(KeyboardKey.KEY_A) == KeyState.KEY_REPEAT)        body_direction = Direction.LEFT;
        else if (Input.GetKeyboardKey(KeyboardKey.KEY_D) == KeyState.KEY_REPEAT)        body_direction = Direction.RIGHT;
        else if (Input.GetKeyboardKey(KeyboardKey.KEY_W) == KeyState.KEY_REPEAT)        body_direction = Direction.TOP;
        else if (Input.GetKeyboardKey(KeyboardKey.KEY_S) == KeyState.KEY_REPEAT)        body_direction = Direction.DOWN;
        else 
        {
            walking = false;
            body_direction = Direction.DOWN;
        }
        head_direction = body_direction;

        shooting = true;
        if      (Input.GetKeyboardKey(KeyboardKey.KEY_LEFT) == KeyState.KEY_REPEAT)     head_direction = Direction.LEFT;
        else if (Input.GetKeyboardKey(KeyboardKey.KEY_RIGHT) == KeyState.KEY_REPEAT)    head_direction = Direction.RIGHT;
        else if (Input.GetKeyboardKey(KeyboardKey.KEY_UP) == KeyState.KEY_REPEAT)       head_direction = Direction.TOP;
        else if (Input.GetKeyboardKey(KeyboardKey.KEY_DOWN) == KeyState.KEY_REPEAT)     head_direction = Direction.DOWN;
        else if (!walking)
        {
            shooting = false;
            head_direction = Direction.DOWN;
        }
        else shooting = false;
        //

        //update collider
        Vec2f vector_player_speed = new Vec2f(0.0f, 0.0f);
        if (Input.GetKeyboardKey(KeyboardKey.KEY_A) == KeyState.KEY_REPEAT)     vector_player_speed.X -= player_speed;
        if (Input.GetKeyboardKey(KeyboardKey.KEY_D) == KeyState.KEY_REPEAT)     vector_player_speed.X += player_speed;
        if (Input.GetKeyboardKey(KeyboardKey.KEY_W) == KeyState.KEY_REPEAT)     vector_player_speed.Y += player_speed;
        if (Input.GetKeyboardKey(KeyboardKey.KEY_S) == KeyState.KEY_REPEAT)     vector_player_speed.Y -= player_speed;
            
        collider.SetSpeed(vector_player_speed);
        //

        //shoot
        fire_rate_timer += dt;
        if (fire_rate_timer >= fire_rate)
        {
            if (shooting)
            {
                Vec2f speed = collider.GetSpeed();
                speed.X *= 0.2f;
                speed.Y *= 0.2f;
                IGameObject spawned_tear = Scene.InstantiatePrefab("Assets/Prefabs/Isaac_Tear_Prefab.prefab");
                Collider2D tear_collider = new Collider2D(spawned_tear);
                tear_collider.SetPosition(transform.GetWorldPosition());

                if      (head_direction == Direction.LEFT)      speed.X -= tear_speed;
                else if (head_direction == Direction.RIGHT)     speed.X += tear_speed;
                else if (head_direction == Direction.TOP)       speed.Y += tear_speed;
                else if (head_direction == Direction.DOWN)      speed.Y -= tear_speed;
                tear_collider.SetSpeed(speed);

                fire_rate_timer = 0.0f;
            }
        }
        //

        // update sprites
        if      (head_direction == Direction.LEFT)      head_animator.PlayAnimation(shooting ? "isaac_head_shooting_left" : "isaac_head_idle_left");
        else if (head_direction == Direction.RIGHT)     head_animator.PlayAnimation(shooting ? "isaac_head_shooting_right" : "isaac_head_idle_right");
        else if (head_direction == Direction.TOP)       head_animator.PlayAnimation(shooting ? "isaac_head_shooting_up" : "isaac_head_idle_up");
        else if (head_direction == Direction.DOWN)      head_animator.PlayAnimation(shooting ? "isaac_head_shooting_down" : "isaac_head_idle_down");

        if      (body_direction == Direction.LEFT)      body_animator.PlayAnimation("isaac_body_walk_left");
        else if (body_direction == Direction.RIGHT)     body_animator.PlayAnimation("isaac_body_walk_right");
        else if (body_direction == Direction.TOP)       body_animator.PlayAnimation("isaac_body_walk_up");
        else if (body_direction == Direction.DOWN)      body_animator.PlayAnimation(walking ? "isaac_body_walk_down" : "isaac_body_idle_down");
        //
    }

    public void GetDamage()
    {
        if (unhittable) return;

        hp -= 1;

        unhittable = true;
        Canvas canvas = new Canvas(canvas_go);
        canvas.SetText("health_ui", hp.ToString());
            
        // die
        if (hp <= 0)
        {
            CallDestroy();
        }
        //get hurt
        else
        {
            audio.PlaySound("isaac_hurt");
        }
    }

    public void CallDestroy()
    {
        audio.PlaySound("isaac_die");
        Scene.DeleteGO(shadow_go);
        Scene.DeleteGO(head_go);
        Scene.DeleteGO(body_go);
        collider.DisableComponent();
        particles.EnableComponent();
        particles.Play();
        Scene.LoadSceneAsync("Assets/Scenes/Prototype3_RogueLike.mlscene", false);
    }

    //functions for colliders
    public override void OnTriggerCollision(IGameObject other)
    {
        if (other.tag == "item")
        {
            other.DisableComponent(ComponentType.Sprite);
            other.tag = "PickedItem";
            audio.PlaySound("item_pickup");
            fire_rate *= 0.5f;
        }
    }
    public override void OnExitCollision(IGameObject other)
    {
    }

    public override void OnTriggerSensor(IGameObject other)
    {
        if (other.tag == "door_up")
        {
            Vec2f spawn_point = new Vec2f();
            spawn_point.X = 0;
            spawn_point.Y = -4;
            collider.SetPosition(spawn_point);
        }
        else if (other.tag == "door_down")
        {
            Vec2f spawn_point = new Vec2f();
            spawn_point.X = 0;
            spawn_point.Y = 4;
            collider.SetPosition(spawn_point);
        }
        else if (other.tag == "door_left")
        {
            Vec2f spawn_point = new Vec2f();
            spawn_point.X = 7;
            spawn_point.Y = 0;
            collider.SetPosition(spawn_point);
        }
        else if (other.tag == "door_right")
        {
            Vec2f spawn_point = new Vec2f();
            spawn_point.X = -7;
            spawn_point.Y = 0;
            collider.SetPosition(spawn_point);
        }
        else if (other.tag == "heart_pickup")
        {
            Scene.DeleteGO(other);
            audio.PlaySound("health_up");
            hp += 1;
            Canvas canvas = new Canvas(canvas_go);
            canvas.SetText("health_ui", hp.ToString());
        }
    }
    public override void OnExitSensor(IGameObject other)
    {
    }
}