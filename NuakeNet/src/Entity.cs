using Coral.Managed.Interop;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Reflection.Metadata;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Nuake.Net
{
    public struct CollisionData
    {
        Entity Entity1;
        Entity Entity2;
        Vector3 Normal;
        Vector3 Position;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Entity
    {
        internal static unsafe delegate*<int, int, bool> EntityHasComponentIcall;
        internal static unsafe delegate*<int, bool> EntityHasManagedInstanceIcall;
        internal static unsafe delegate*<int, NativeString, int> EntityGetEntityIcall;
        internal static unsafe delegate*<int, NativeString> EntityGetNameIcall;
        internal static unsafe delegate*<int, NativeString, void> EntitySetNameIcall;
        internal static unsafe delegate*<int, bool> EntityIsValidIcall;
        internal static unsafe delegate*<NativeString, NativeArray<int>> EntityGetTargetsIcall;
        internal static unsafe delegate*<int, NativeString> EntityGetTargetIcall;

        public enum ComponentTypes
        {
            Unknown = -1,
            PARENT = 0,
            NAME,
            PREFAB,
            TRANSFORM,
            LIGHT,
            CAMERA,
            AUDIO_EMITTER,
            MODEL,
            SKINNED_MODEL,
            BONE,
            RIGIDBODY,
            BOX_COLLIDER,
            SPHERE_COLLIDER,
            CAPSULE_COLLIDER,
            CYLINDER_COLLIDER,
            MESH_COLLIDER,
            CHARACTER_CONTROLLER,
            PARTICLE_EMITTER,
            QUAKE_MAP,
            BSP_BRUSH,
            SPRITE,
            NAVMESH
        }

        public List<Entity> Targets
        { 
            get 
            {
                unsafe 
                {
                    var targetIds = EntityGetTargetsIcall(Target);

                    List<Entity> targets = new List<Entity>();
                    foreach(var target in targetIds)
                    {
                        bool hasInstance = EntityHasManagedInstanceIcall(target);
                        Entity entityInstance;
                        if (hasInstance)
                        {
                            entityInstance = Scene.GetEntity<Entity>(target);
                        }
                        else
                        {
                            entityInstance = new Entity(ECSHandle);
                        }

                        targets.Add(entityInstance);
                    }

                    return targets; 
                }
            }
        }

        public string Target { get { unsafe { return EntityGetTargetIcall(ECSHandle);  } }}

        public int ID { get; set; }
        public int ECSHandle { get; set; } = -1;

        public string Name
        {
            get
            {
                unsafe
                {
                    NativeString name = EntityGetNameIcall(ECSHandle);
                    return name;
                }
            }
            set
            {
                if(String.IsNullOrEmpty(value))
                {
                    Engine.Log("Invalid entity name: cannot be empty.");
                    return;
                }

                unsafe
                {
                    EntitySetNameIcall(ECSHandle, value);
                }
            }
        }

        public Entity() { }
        public Entity(int handle)
        {
            ECSHandle = handle;
        }

        public virtual void OnInit() { }
        public virtual void OnUpdate(float dt) { }
        public virtual void OnFixedUpdate(float dt) { }
        public virtual void OnDestroy() { }

        public virtual void OnCollision(Entity entity) 
        {
        }

        public virtual void Activate(Entity triggeredFrom)
        {

        }

        // Physics
        public void OnCollisionInternal(int entity) 
        {
            Entity entityInstance;
            unsafe
            {
                bool hasInstance = EntityHasManagedInstanceIcall(entity);

                if (hasInstance)
                {
                    entityInstance = Scene.GetEntity<Entity>(entity);
                }
                else
                {
                    entityInstance = new Entity(ECSHandle);
                }
            }

            OnCollision(entityInstance);
        }

        protected static Dictionary<Type, ComponentTypes> MappingTypeEnum = new Dictionary<Type, ComponentTypes>()
        {
            { typeof(ParentComponent), ComponentTypes.PARENT },
            { typeof(NameComponent), ComponentTypes.NAME },
            { typeof(PrefabComponent), ComponentTypes.PREFAB },
            { typeof(TransformComponent), ComponentTypes.TRANSFORM},
            { typeof(LightComponent), ComponentTypes.LIGHT },
            { typeof(CameraComponent), ComponentTypes.CAMERA },
            { typeof(AudioEmitterComponent), ComponentTypes.AUDIO_EMITTER },
            { typeof(ModelComponent), ComponentTypes.MODEL },
            { typeof(SkinnedModelComponent), ComponentTypes.SKINNED_MODEL },
            { typeof(BoneComponent), ComponentTypes.BONE },
            { typeof(BoxCollider), ComponentTypes.BOX_COLLIDER },
            { typeof(SphereCollider), ComponentTypes.SPHERE_COLLIDER },
            { typeof(CapsuleColliderComponent), ComponentTypes.CAPSULE_COLLIDER },
            { typeof(CylinderColliderComponent), ComponentTypes.CYLINDER_COLLIDER },
            { typeof(MeshColliderComponent), ComponentTypes.MESH_COLLIDER },
            { typeof(CharacterControllerComponent), ComponentTypes.CHARACTER_CONTROLLER },
            { typeof(ParticleEmitterComponent), ComponentTypes.PARTICLE_EMITTER },
            { typeof(QuakeMapComponent), ComponentTypes.QUAKE_MAP },
            { typeof(BSPBrushComponent), ComponentTypes.BSP_BRUSH },
            { typeof(SpriteComponent), ComponentTypes.SPRITE },
            { typeof(NavMeshVolumeComponent), ComponentTypes.NAVMESH }
        };

        public bool HasComponent<T>()
        {
            if (MappingTypeEnum.ContainsKey(typeof(T)))
            {
                unsafe { return EntityHasComponentIcall(ECSHandle, (int)MappingTypeEnum[typeof(T)]); };
            }

            return false;
        }

        public T? GetComponent<T>() where T : IComponent
        {
            if (HasComponent<T>())
            {
                return (T?)Activator.CreateInstance(typeof(T), ECSHandle);
            }

            throw new Exception("Component not found: " + typeof(T).GetType().Name);
        }

        public T? GetEntity<T>(string path) where T : Entity
        {
            Entity entityInstance;
            unsafe
            {
                int entityHandle = EntityGetEntityIcall(ECSHandle, path );
                 
                bool hasInstance = EntityHasManagedInstanceIcall(entityHandle);
                if (hasInstance)
                {
                    entityInstance = Scene.GetEntity<Entity>(entityHandle);
                }
                else
                {
                    entityInstance = new Entity(entityHandle);
                }
            }

            return entityInstance as T;
        }

        public static implicit operator bool(Entity entity)
        {
            if (object.ReferenceEquals(entity, null))
            {
                return false;
            }

            bool isValid;
            unsafe
            {
                isValid = EntityIsValidIcall(entity.ECSHandle);
            }

            return isValid;
        }
    }

    public class Prefab
    {
        internal static unsafe delegate*<NativeString, Vector3, float, float, float, float, int> PrefabInstanceIcall;

        string Path {  get; set; }

        public Prefab()
        {

        }

        public Prefab(string path)
        {
            Path = path;
            Engine.Log("Prefab instance created with path: " + path);
        }

        public Entity? Instance(Vector3 position = default, Quaternion quat = default)
        {
            int handle;
            unsafe { handle = PrefabInstanceIcall(Path, position, quat.X, quat.Y, quat.Z, quat.W); }

            if (handle == -1)
            {
                return null;
            }

            Entity entity = new Entity(handle);
            return entity;
        }

        public static implicit operator bool(Prefab prefab)
        {
            if (object.ReferenceEquals(prefab, null))
            {
                return false;
            }

            return prefab.Path != string.Empty;
        }
    }
}
