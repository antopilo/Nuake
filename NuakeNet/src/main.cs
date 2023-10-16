using Coral.Managed.Interop;

using System;
using System.Collections.Generic;

namespace Nuake.Net
{
    /// <summary>
    /// This is the core Nuake.Net API. 
    /// All internal call should happen in this file.
    /// </summary>

    public class Engine
    {
        internal static unsafe delegate*<NativeString, void> LoggerLogIcall;

        public Engine() { }

        /// <summary>
        /// Prints a message to the console log
        /// </summary>
        /// <param name="message">message to be printed</param>
        public static void Log(string input)
        {
            unsafe { LoggerLogIcall(input); }
        }
    }

    public class IComponent
    {
        public UInt32 EntityID { get; protected set; }
    }

    public class  LightComponent : IComponent
    {
        public LightComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public float Intensity { get; set; }
    }

    public class TransformComponent : IComponent
    {
        public TransformComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public int LocalPosition { get; set; }
        public int GlobalPosition { get; set; }


        public void SetGlobalPosition() { }
        public void GetGlobalPosition() { }
    }

    
    public class Entity
    {
        internal static unsafe delegate*<UInt32, NativeString, bool> EntityHasComponentIcall;

        public UInt32 ID { get; private set; }

        public virtual void OnInit() { }
        public virtual void OnUpdate(float dt) { }
        public virtual void OnFixedUpdate(float dt) { }
        public virtual void OnDestroy() { }

        public bool HasComponent<T>()
        {
            if (typeof(T) == typeof(TransformComponent))
            {
                unsafe { return EntityHasComponentIcall(ID, typeof(TransformComponent).GetType().Name); };
            }
            else if (typeof(T) == typeof(LightComponent))
            {
                return true;
            }

            return false;
        }

        public T GetComponent<T>()
        {
            if (typeof(T) == typeof(TransformComponent))
            {
                return (T)(object)new TransformComponent(ID);
            }
            else
            {
                throw new InvalidOperationException("Component not found");
            }
        }
    }

    public class Scene
    {
        public UInt32 GetEntityIDByName(string name)
        {
            return 0;
        }
    }
}