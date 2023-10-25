using Coral.Managed.Interop;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Nuake.Net
{
    public class IComponent
    {
        public int EntityID { get; protected set; }
    }
    public class ParentComponent : IComponent
    {
        public ParentComponent(UInt32 entityId) { }

        public Entity Parent { get; set; }
        public List<Entity> Children { get; set; }
    }
    public class NameComponent : IComponent
    {
        public NameComponent(int entityId)
        {
            EntityID = entityId;
        }

        public string Name { get; set; }
    }

    public class PrefabComponent : IComponent
    {
        public PrefabComponent(UInt32 entityId) { }

        public string Prefab { get; set; }
    }

    public class TransformComponent : IComponent
    {
        internal static unsafe delegate*<int, float, float, float, void> SetPositionIcall;
        internal static unsafe delegate*<int, float, float, float, void> RotateIcall;

        public TransformComponent(int entityId)
        {
            EntityID = entityId;
        }

        /// <summary>
        /// Rotates the transform using euler angles
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <param name="z"></param>
        public void Rotate(float x, float y, float z)
        {
            unsafe { RotateIcall(EntityID, x, y, z); }
        }

        public Vector3 Rotation { get; set; }
        public Vector3 GlobalRotation 
        { 
            get { return new Vector3(); }
            set
            {
                
            } 
        }

        public Vector3 LocalPosition
        {
            get { return new Vector3(); }
            set
            {
                unsafe { SetPositionIcall(EntityID, value.X, value.Y, value.Z); }
            }
        }
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

        public LightComponent(int entityId)
        {
            EntityID = entityId;
        }

        public LightType Type { get; set; }
        public float Intensity { get; set; }
        public Color Color { get; set; }
        public bool CastShadows { get; set; }
        public bool Volumetric { get; set; }
        public bool SyncWithSky { get; set; }
    }

    public class CameraComponent : IComponent
    {
        internal static unsafe delegate*<int, NativeArray<float>> GetDirectionIcall;

        public CameraComponent(int entityId)
        {
            EntityID = entityId;
        }

        public float FOV { get; set; }

        public Vector3 Direction
        {
            get
            {
                unsafe 
                { 
                    NativeArray<float> direction = GetDirectionIcall(EntityID);
                    return new Vector3(direction[0], direction[1], direction[2]);
                }
            }
        }

    }

    public class AudioEmitterComponent : IComponent
    {
        public AudioEmitterComponent(int entityId)
        {
            EntityID = entityId;
        }

        public string AudioClip { get; set; }

        public bool Loop { get; set; }
        public bool Spatialized { get; set; }
        public bool Playing { get; set; }
    }

    public class ModelComponent : IComponent
    {
        public ModelComponent(int entityId)
        {
            EntityID = entityId;
        }

        public string Mesh { get; set; }
    }

    public class SkinnedModelComponent : IComponent
    {
        public SkinnedModelComponent(int entityId) { }

        public bool Playing { get; set; }
        public int CurrentAnimation { get; set; }
    }

    public class BoneComponent : IComponent
    {
        public BoneComponent(int entityId)
        {
            EntityID = entityId;
        }
        public string Name { get; set; }

    }
    public class RigidbodyComponent : IComponent
    {
        public RigidbodyComponent(int entityId) { }

        public float Mass { get; set; }
    }

    public class BoxCollider : IComponent
    {
        public BoxCollider(int entityId)
        {
            EntityID = entityId;
        }

        public Vector3 Scale { get; set; }
    }

    public class SphereCollider : IComponent
    {
        public SphereCollider(int entityId)
        {
            EntityID = entityId;
        }

        public float Radius { get; set; }
    }


    public class CapsuleColliderComponent : IComponent
    {
        public CapsuleColliderComponent(int entityId)
        {
            EntityID = entityId;
        }

        public float Radius { get; set; }
        public float Height { get; set; }
    }
    public class CylinderColliderComponent : IComponent
    {
        public CylinderColliderComponent(int entityId)
        {
            EntityID = entityId;
        }

        public float Radius { get; set; }
        public float Height { get; set; }
    }
    public class MeshColliderComponent : IComponent
    {
        public MeshColliderComponent(int entityId)
        {
            EntityID = entityId;
        }

        public int Mesh { get; set; }
    }

    public class CharacterControllerComponent : IComponent
    {
        internal static unsafe delegate*<int, float, float, float, void> MoveAndSlideIcall;
        internal static unsafe delegate*<int, bool> IsOnGroundIcall;

        public CharacterControllerComponent(int entityId)
        {
            EntityID = entityId;
        }

        public void MoveAndSlide(Vector3 velocity)
        {
            unsafe { MoveAndSlideIcall(EntityID, velocity.X, velocity.Y, velocity.Z); }
        }

        public bool IsOnGround()
        {
            unsafe { return IsOnGroundIcall(EntityID); }
        }
    }

    public class ParticleEmitterComponent : IComponent
    {
        public ParticleEmitterComponent(int entityId) { }

        public float Amount { get; set; }
        public float Life { get; set; }
        public Vector3 Gravity { get; set; }
    }
    public class QuakeMapComponent : IComponent
    {
        public QuakeMapComponent(int entityId) { }

        public string Map { get; set; }

        public bool BuildCollisions { get; set; }

        public void Build() { }
    }
    public class BSPBrushComponent : IComponent
    {
        public BSPBrushComponent(int entityId)
        {
            EntityID = entityId;
        }
    }
    public class SpriteComponent : IComponent
    {
        public SpriteComponent(int entityId) { }

        public string Sprite { get; set; }
    }
}
