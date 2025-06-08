using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;

public class MonoBehaviour
{
    private static Dictionary<IGameObject, Dictionary<Type, MonoBehaviour>> _components =
    new Dictionary<IGameObject, Dictionary<Type, MonoBehaviour>>();


    public IGameObject attached_game_object;
    public MonoBehaviour() : base()
    {
        attached_game_object = new IGameObject();
        RegisterComponent();
    }

    public object GetClassInstance()
    {
        return this;
    }

    public virtual void Start() { }
    public virtual void Update() { }


    private void RegisterComponent()
    {
        if (!_components.ContainsKey(attached_game_object))
        {
            _components[attached_game_object] = new Dictionary<Type, MonoBehaviour>();
        }

        Type componentType = this.GetType();
        _components[attached_game_object][componentType] = this;
    }

    public static T GetComponent<T>(IGameObject go) where T : MonoBehaviour
    {
        if (_components.TryGetValue(go, out var components))
        {
            Type type = typeof(T);
            if (components.TryGetValue(type, out var component))
            {
                return (T)component;
            }
        }
        return null;
    }

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