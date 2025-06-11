
using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class PongBall : MonoBehaviour
{
    //create variables here
    public IGameObject pad_particles_go;
    public IGameObject wall_particles_go;
    public IGameObject win_particles_go;

    private Transform pad_particles_t;
    private Transform wall_particles_t;
    private Transform win_particles_t;

    private Particles pad_particles;
    private Particles wall_particles;
    private Particles win_particles;

    private Collider2D ball_collider;
    private Transform ball_t;
    private Audio ball_audio;

    public IGameObject game_manager_go;
    private PongGameManager game_manager_script;

    public float ball_speed = 10.0f;
    private float ball_current_speed;

    private float respawn_ball_timer;

    //will be called once
    public override void Start()
    {
        pad_particles_t = new Transform(pad_particles_go);
        wall_particles_t = new Transform(wall_particles_go);
        win_particles_t = new Transform(win_particles_go);

        pad_particles = new Particles(pad_particles_go);
        wall_particles = new Particles(wall_particles_go);
        win_particles = new Particles(win_particles_go);

        ball_collider = new Collider2D(attached_game_object);
        ball_t = new Transform(attached_game_object);
        ball_audio = new Audio(attached_game_object);

        game_manager_script = MonoBehaviour.GetComponent<PongGameManager>(game_manager_go);

        Vec2f speed = new Vec2f(0.0f, 0.0f);
        ball_collider.SetSpeed(speed);

        ball_current_speed = ball_speed;
    }

    public void StartBall()
    {
        respawn_ball_timer = 0.0f;
        ball_current_speed = ball_speed;
        Vec2f speed = new Vec2f(1.0f, 0.70f);
        speed.Normalize();
        speed = speed * ball_current_speed;
        ball_collider.SetSpeed(speed);
    }

    //it is called every frame
    public override void Update()
    {
        float dt = ML_System.GetDT();
        respawn_ball_timer += dt;
        if (respawn_ball_timer >= 5.0f)
        {
            game_manager_script.PlayerScored(true, true);
            respawn_ball_timer = 0.0f;
        }

        Vec2f speed = ball_collider.GetSpeed();
        speed.Normalize();
        speed = speed * ball_current_speed;
        ball_collider.SetSpeed(speed);
    }

    //functions for colliders
    public override void OnTriggerCollision(IGameObject other)
    {
        if (other.tag == "TopWall")
        {
            Vec2f particle_pos = ball_t.GetWorldPosition();
            particle_pos.Y = 5.1f;
            wall_particles_t.SetWorldPosition(particle_pos);
            wall_particles_t.SetWorldAngle(180.0f);
            wall_particles.Play();
            ball_audio.PlaySound("pong_ball_hit_wall");
        }
        else if (other.tag == "BottomWall")
        {
            Vec2f particle_pos = ball_t.GetWorldPosition();
            particle_pos.Y = -5.1f;
            wall_particles_t.SetWorldPosition(particle_pos);
            wall_particles_t.SetWorldAngle(0.0f);
            wall_particles.Play();
            ball_audio.PlaySound("pong_ball_hit_wall");
        }
        else if (other.tag == "LeftPad")
        {
            Vec2f particle_pos = ball_t.GetWorldPosition();
            particle_pos.X = -8.8f;
            pad_particles_t.SetWorldPosition(particle_pos);
            pad_particles_t.SetWorldAngle(0.0f);
            pad_particles.Play();
            ball_current_speed += 0.5f;
            respawn_ball_timer = 0.0f;
            ball_audio.PlaySound("pong_ball_hit_pad");
        }
        else if (other.tag == "RightPad")
        {
            Vec2f particle_pos = ball_t.GetWorldPosition();
            particle_pos.X = 8.8f;
            pad_particles_t.SetWorldPosition(particle_pos);
            pad_particles_t.SetWorldAngle(180.0f);
            pad_particles.Play();
            ball_current_speed += 0.5f;
            respawn_ball_timer = 0.0f;
            ball_audio.PlaySound("pong_ball_hit_pad");
        }
    }
    public override void OnExitCollision(IGameObject other)
    {
    }

    public override void OnTriggerSensor(IGameObject other)
    {
        if (other.tag == "LeftWall")
        {
            Vec2f particle_pos = ball_t.GetWorldPosition();
            particle_pos.X = -10.1f;
            win_particles_t.SetWorldPosition(particle_pos);
            win_particles_t.SetWorldAngle(0.0f);
            win_particles.Play();
            ball_audio.PlaySound("pong_ball_explosion");
            game_manager_script.PlayerScored(false, false);
        }
        else if (other.tag == "RightWall")
        {
            Vec2f particle_pos = ball_t.GetWorldPosition();
            particle_pos.X = 10.1f;
            win_particles_t.SetWorldPosition(particle_pos);
            win_particles_t.SetWorldAngle(180.0f);
            win_particles.Play();
            ball_audio.PlaySound("pong_ball_explosion");
            game_manager_script.PlayerScored(true, false);
        }
    }
    public override void OnExitSensor(IGameObject other)
    {
    }
}