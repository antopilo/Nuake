using Coral.Managed.Interop;

using System;
using System.Collections.Generic;
using System.Drawing;

namespace Nuake.Net
{
    /// <summary>
    /// This is the core Nuake.Net API. 
    /// All internal call should happen in this file.
    /// </summary>

    public class Vector3
    {
        public float x { get; set; }
        public float y { get; set; }
        public float z { get; set; }

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        // TODO: Add operator
    }
    public class Quat 
    {
        public float x { get; set; }
        public float y { get; set; }
        public float z { get; set; }
        public float w { get; set; }

        public Quat(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }
    }

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



    public class TransformComponent : IComponent
    {
        public TransformComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public Vector3 LocalPosition { get; set; }
        public Vector3 GlobalPosition { get; set; }
    }

    public class LightComponent : IComponent
    {
        public enum LightType
        {
            Directional,
            Point,
            Spot
        }

        public LightComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public LightType Type { get; set; }
        public float Intensity { get; set; }
        public Color Color { get; set; }
        public bool CastShadows {  get; set; }
        public bool Volumetric { get; set; }
        public bool SyncWithSky { get; set; }

    }

    public class CameraComponent : IComponent
    {
        public CameraComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public float FOV { get; set; }
    }

    public class AudioEmitter : IComponent
    {
        public AudioEmitter(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public string AudioClip { get; set; }

        public bool Loop { get; set; }
        public bool Spatialized { get; set; }
        public bool Playing { get; set; }
    }

    public class BoneComponent : IComponent
    {
        public BoneComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }
        public string Name { get; set; }

    }

    public class BoxCollider : IComponent
    {
        public BoxCollider(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public Vector3 Scale { get; set; }
    }

    public class BSPBrushComponent : IComponent
    {
        public BSPBrushComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }
    }

    public class CapsuleColliderComponent : IComponent
    {
        public CapsuleColliderComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public float Radius { get; set; }
        public float Height { get; set; }
    }

    public class CharacterControllerComponent : IComponent
    {
        public CharacterControllerComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public void MoveAndSlide(Vector3 velocity)
        {

        }

        public bool IsOnGround()
        {
            return false;
        }
    }

    public class CylinderColliderComponent : IComponent
    {
        public CylinderColliderComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public float Radius { get; set; }
        public float Height { get; set; }
    }

    public class MeshColliderComponent : IComponent
    {
        public MeshColliderComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public int Mesh { get; set; }
    }

    public class ModelComponent : IComponent
    {
        public ModelComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public string Mesh { get; set; }
    }

    public class NameComponent : IComponent
    {
        public NameComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public string Name { get; set; }
    }

    public class ParentComponent : IComponent
    {
        public ParentComponent(UInt32 entityId) { }

        public Entity Parent{ get; set; }
        public List<Entity> Children { get; set;}
    }

    public class ParticleEmitterComponent : IComponent
    {
        public ParticleEmitterComponent(UInt32 entityId) { }

        public float Amount { get; set; }
        public float Life {  get; set; }
        public Vector3 Gravity { get; set; }
    }

    public class PrefabComponent : IComponent
    {
        public PrefabComponent(UInt32 entityId) { }

        public string Prefab { get; set; }
    }

    public class QuakeMapComponent : IComponent
    {
        public QuakeMapComponent(UInt32 entityId) { }

        public string Map { get; set; }

        public bool BuildCollisions {  get; set; }

        public void Build() { }
    }

    public class RigidbodyComponent : IComponent
    {
        public RigidbodyComponent(UInt32 entityId) { }

        public float Mass {  get; set; }
    }

    public class SkinnedModelComponent : IComponent
    {
        public SkinnedModelComponent(UInt32 entityId) { }

        public bool Playing { get; set; }
        public int CurrentAnimation { get; set; }
    }

    public class SphereColliderComponent : IComponent
    {
        public SphereColliderComponent(UInt32 entityId) { }

        public float Radius { get; set; }
    }

    public class SpriteComponent : IComponent
    {
        public SpriteComponent(UInt32 entityId) { }

        public string Sprite { get; set; }
    }

    public class WrenScriptComponent : IComponent
    {
        public WrenScriptComponent(UInt32 entityId) { }

        public string Script { get; set; }
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