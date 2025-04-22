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

    //since transformation from cpp to csharp uses intptr
    //we first call its ptr version, this will create an
    //IGameObject out of the ptr and call its regular and
    //usable version
    public virtual void OnTriggerCollisionPtr(IntPtr other)
    {
        IGameObject otherGO = new IGameObject(other);
        OnTriggerCollision(otherGO);
    }
    public virtual void OnTriggerSensorPtr(IntPtr other)
    {
        IGameObject otherGO = new IGameObject(other);
        OnTriggerSensor(otherGO);
    }
    public virtual void OnExitCollisionPtr(IntPtr other)
    {
        IGameObject otherGO = new IGameObject(other);
        OnExitCollision(otherGO);
    }
    public virtual void OnExitSensorPtr(IntPtr other)
    {
        IGameObject otherGO = new IGameObject(other);
        OnExitSensor(otherGO);
    }

    public virtual void OnTriggerCollision(IGameObject other) { }
    public virtual void OnTriggerSensor(IGameObject other) { }
    public virtual void OnExitCollision(IGameObject other) { }
    public virtual void OnExitSensor(IGameObject other) { }
}