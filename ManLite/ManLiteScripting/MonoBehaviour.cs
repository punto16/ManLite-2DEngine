using ManLiteScripting;
using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;

public class MonoBehaviour
{
    public IGameObject attached_game_object;
    public MonoBehaviour() : base() { attached_game_object = new IGameObject(); }

    public object GetClassInstance()
    {
        return this;
    }

    public virtual void Start() { }
    public virtual void Update() { }

    public virtual void OnTriggerCollision(IGameObject other) { }
    public virtual void OnTriggerSensor(IGameObject other) { }
    public virtual void OnExitCollision(IGameObject other) { }
    public virtual void OnExitSensor(IGameObject other) { }
}