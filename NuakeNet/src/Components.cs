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
        internal static unsafe delegate*<int, NativeString> GetNameIcall;

        public NameComponent(int entityId)
        {
            EntityID = entityId;
        }

        public string Name 
        {
            get
            {
                unsafe { return GetNameIcall(EntityID).ToString(); }
            }
            set
            {

            }
        }
    }

    public class PrefabComponent : IComponent
    {
        public PrefabComponent(UInt32 entityId) { }

        public string Prefab { get; set; }
    }

    public class TransformComponent : IComponent
    {
        internal static unsafe delegate*<int, NativeArray<float>> GetGlobalPositionIcall;
        internal static unsafe delegate*<int, NativeArray<float>> GetPositionIcall;
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
            get 
            {
                unsafe
                {
                    NativeArray<float> result = GetPositionIcall(EntityID);
                    return new Vector3(result[0], result[1], result[2]);
                }
            }
            set
            {
                unsafe { SetPositionIcall(EntityID, value.X, value.Y, value.Z); }
            }
        }
        public Vector3 GlobalPosition 
        {
            get
            {
                unsafe 
                { 
                    NativeArray<float> result = GetGlobalPositionIcall(EntityID); 
                    return new Vector3(result[0], result[1], result[2]);
                }
            }
            set { }
        }
    }

    public class LightComponent : IComponent
    {
        internal static unsafe delegate*<int, float> GetLightIntensityIcall;
        internal static unsafe delegate*<int, float, void> SetLightIntensityIcall;

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
        public float Intensity
        {

            get 
            {
                unsafe
                {
                    return GetLightIntensityIcall(EntityID); 
                }
            }
            set 
            {
                unsafe
                {
                    SetLightIntensityIcall(EntityID, value);
                }
            }
        }

        public Color Color { get; set; }
        public bool CastShadows { get; set; }
        public bool Volumetric { get; set; }
        public bool SyncWithSky { get; set; }
    }

    public class CameraComponent : IComponent
    {
        internal static unsafe delegate*<int, NativeArray<float>> GetDirectionIcall;
        internal static unsafe delegate*<int, float, void> SetCameraFOVIcall;
        internal static unsafe delegate*<int, float> GetCameraFOVIcall;

        public CameraComponent(int entityId)
        {
            EntityID = entityId;
        }

        public float FOV 
        {
            get
            {
                unsafe { return GetCameraFOVIcall(EntityID); }
            }
            set
            {
                unsafe { SetCameraFOVIcall(EntityID, value); }
            }
        }

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
        internal static unsafe delegate*<int, bool> GetIsPlayingIcall;
        internal static unsafe delegate*<int, bool, void> SetIsPlayingIcall;

        public AudioEmitterComponent(int entityId)
        {
            EntityID = entityId;
        }

        public string AudioClip { get; set; }

        public bool Loop { get; set; }
        public bool Spatialized { get; set; }
        public bool Playing 
        {
            get { unsafe { return GetIsPlayingIcall(EntityID); } }
            set { unsafe { SetIsPlayingIcall(EntityID, value); } }
        }
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
        internal static unsafe delegate*<int, NativeString, void> PlayIcall;

        public SkinnedModelComponent(int entityId) 
        {
            EntityID = entityId;
        }

        public bool Playing { get; set; }
        public int CurrentAnimation { get; set; }

        public void Play(String name)
        {
            unsafe { PlayIcall(EntityID, name); }
        }
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
        internal static unsafe delegate*<int, NativeArray<float>> GetGroundVelocityIcall;
        internal static unsafe delegate*<int, NativeArray<float>> GetGroundNormalIcall;

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

        public Vector3 GetGroundNormal()
        {
            unsafe
            {
                NativeArray<float> resultArray = GetGroundNormalIcall(EntityID);
                return new(resultArray[0], resultArray[1], resultArray[2]);
            }
        }

        public Vector3 GetGroundVelocity()
        {
            unsafe
            {
                NativeArray<float> resultArray = GetGroundVelocityIcall(EntityID);
                return new(resultArray[0], resultArray[1], resultArray[2]);
            }
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

    public class NavMeshVolumeComponent : IComponent
    {
        internal static unsafe delegate*<int, float, float, float, float, float, float, NativeArray<float>> FindPathIcall;

        public NavMeshVolumeComponent(int entityId) {  EntityID = entityId; }

        public List<Vector3> FindStraightPath(Vector3 start, Vector3 end)
        {
            List<Vector3> pathWaypoints = [];
            unsafe
            {
                NativeArray<float> waypoints = FindPathIcall(EntityID, start.X, start.Y, start.Z, end.X, end.Y, end.Z);
                for(int i = 0; i < waypoints.Length; i += 3)
                {
                    Vector3 waypointPosition = new Vector3(waypoints[i], waypoints[i + 1], waypoints[i + 2]);
                    pathWaypoints.Add(waypointPosition);
                }

                return pathWaypoints;
            }
        }
    }

}
