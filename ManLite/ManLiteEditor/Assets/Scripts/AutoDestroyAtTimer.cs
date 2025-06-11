
using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class AutoDestroyAtTimer : MonoBehaviour
{
    //create variables here
    public float seconds_to_be_destroyed;
    private float timer = 0.0f;

    private bool destroying = false;

    private IGameObject shadow_go;

    //components
    private Particles destroy_particles;
    private Audio audio_source;

    //will be called once
    public override void Start()
    {
        shadow_go = attached_game_object.GetChildGO("isaac_tear_shadow");
        destroy_particles = new Particles(attached_game_object);
        audio_source = new Audio(attached_game_object);
        timer = 0.0f;
        destroying = false;
    }

    //it is called every frame
    public override void Update()
    {
        timer += ML_System.GetDT();

        //destroy animation
        if (!destroying && timer >= seconds_to_be_destroyed)
        {
            CallDestroy();
        }
        //destroy for real
        if (destroying && timer >= 1.0f)
        {
            Scene.DeleteGO(attached_game_object);
            timer = 0.0f;
            destroying = false;
        }
    }

    public void CallDestroy()
    {
        attached_game_object.DisableComponent(ComponentType.Collider2D);
        attached_game_object.DisableComponent(ComponentType.Sprite);
        if (shadow_go) Scene.DeleteGO(shadow_go);
        destroy_particles.EnableComponent();
        destroy_particles.Play();
        audio_source.PlaySound("tear_drop");

        timer = 0.0f;
        destroying = true;
    }

    //functions for colliders
    public override void OnTriggerCollision(IGameObject other)
    {
    }
    public override void OnExitCollision(IGameObject other)
    {
    }

    public override void OnTriggerSensor(IGameObject other)
    {
        if (other.tag == "wall" ||
            other.tag == "door_down" || other.tag == "door_left" ||
            other.tag == "door_right" || other.tag == "door_up")
        {
            CallDestroy();
        }
    }
    public override void OnExitSensor(IGameObject other)
    {
    }
}