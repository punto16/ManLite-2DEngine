using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ManLiteScripting
{
    public class IGameObject
    {
        public IntPtr game_object_ptr = IntPtr.Zero;

        public bool IsValid => game_object_ptr != IntPtr.Zero;

        public string name
        {
            get
            {
                return InternalCalls.GetGOName(game_object_ptr);
            }
            set
            {
                InternalCalls.SetGOName(game_object_ptr, value);
            }
        }

        public bool enabled
        {
            get
            {
                return InternalCalls.GetGOEnabled(game_object_ptr);
            }
            set
            {
                InternalCalls.SetGOEnabled(game_object_ptr, value);
            }
        }

        public bool visible
        {
            get
            {
                return InternalCalls.GetGOVisible(game_object_ptr);
            }
            set
            {
                InternalCalls.SetGOVisible(game_object_ptr, value);
            }
        }

        public string tag
        {
            get
            {
                return InternalCalls.GetGOTag(game_object_ptr);
            }
            set
            {
                InternalCalls.SetGOTag(game_object_ptr, value);
            }
        }

        public uint id
        {
            get
            {
                return InternalCalls.GetGOID(game_object_ptr);
            }
        }

        public IGameObject()
        {
            game_object_ptr = InternalCalls.GetGOPtr();
        }

        public IGameObject(IntPtr go)
        {
            game_object_ptr = go;
        }

        public static implicit operator bool(IGameObject go)
        {
            return go?.IsValid ?? false;
        }

        public bool Exists()
        {
            return (bool)this;
        }

        public static bool operator ==(IGameObject a, IGameObject b) => a?.game_object_ptr == b?.game_object_ptr;
        public static bool operator !=(IGameObject a, IGameObject b) => !(a == b);
        public override bool Equals(object obj) => obj is IGameObject other && this == other;
        public override int GetHashCode() => game_object_ptr.GetHashCode();
    }
}
