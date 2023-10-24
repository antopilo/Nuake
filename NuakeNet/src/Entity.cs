using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Nuake.Net
{
    public class Entity
    {
        internal static unsafe delegate*<int, int, bool> EntityHasComponentIcall;

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
            SPRITE
        }

        public int ID { get; set; }
        public int ECSHandle { get; set; }
        public virtual void OnInit() { }
        public virtual void OnUpdate(float dt) { }
        public virtual void OnFixedUpdate(float dt) { }
        public virtual void OnDestroy() { }

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
            { typeof(SpriteComponent), ComponentTypes.SPRITE }
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

            return null;
        }
    }
}
