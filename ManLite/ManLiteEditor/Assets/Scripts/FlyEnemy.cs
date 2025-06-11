
using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class FlyEnemy : MonoBehaviour
{
    //create variables here
    public int hp = 3;
    public float fly_speed = 1.0f;
    public IGameObject player;

    private IGameObject fly_shadow;
    private IGameObject fly_sprite;

    private Transform player_transform;
    private Transform fly_transform;
    private Collider2D fly_collider;
    private Collider2D player_collider;
    private Audio fly_audio;
    private Particles fly_die_particles;
    
    private string isaac_tear_tag = "isaac_tear";
    private string isaac_tag = "player";

    private float destroy_timer = 0.0f;

    //will be called once
    public override void Start()
    {
        fly_shadow = attached_game_object.GetChildGO("fly_sprite_shadow");
        fly_sprite = attached_game_object.GetChildGO("sprite_fly");
        player_transform = new Transform(player);
        fly_transform = new Transform(attached_game_object);
        fly_collider = new Collider2D(attached_game_object);
        player_collider = new Collider2D(player);
        fly_die_particles = new Particles(attached_game_object);
        fly_audio = new Audio(attached_game_object);
        destroy_timer = 0.0f;
    }

    //it is called every frame
    public override void Update()
    {
        if (hp > 0)
        {
            Vec2f direction = player_transform.GetWorldPosition() - fly_transform.GetWorldPosition();
            direction.Normalize();
            direction = direction * fly_speed;
            fly_collider.SetSpeed(direction);
        }
        else
        {
            destroy_timer += ML_System.GetDT();
            if (destroy_timer >= 1.0f)
            {
                Scene.DeleteGO(attached_game_object);
                destroy_timer = 0.0f;
            }
        }
    }

    public void CallDestroy()
    {
        Scene.DeleteGO(fly_shadow);
        Scene.DeleteGO(fly_sprite);
        attached_game_object.DisableComponent(ComponentType.Collider2D);
        fly_die_particles.EnableComponent();
        fly_die_particles.Play();
        fly_audio.PlaySound("pop_sound");
    }

    //functions for colliders
    public override void OnTriggerCollision(IGameObject other)
    {
        if (other.tag == isaac_tag)
        {
            Isaac_Script isaac_script = MonoBehaviour.GetComponent<Isaac_Script>(other);
            if (isaac_script != null)
            {
                isaac_script.GetDamage();
            }
        }
    }
    public override void OnExitCollision(IGameObject other)
    {
    }

    public override void OnTriggerSensor(IGameObject other)
    {
        if (other.tag == isaac_tear_tag)
        {
            hp -= 1;
            if (hp <= 0)
            {
                CallDestroy();
            }
            AutoDestroyAtTimer tear_script = MonoBehaviour.GetComponent<AutoDestroyAtTimer>(other);
            tear_script.CallDestroy();
        }
    }
    public override void OnExitSensor(IGameObject other)
    {
    }
}