using Coral.Managed.Interop;

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Numerics;
using System.Runtime.InteropServices;

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

    // Input
    public enum Key
    {
         SPACE          =   32,
         APOSTROPHE     =   39, 
         COMMA          =   44, 
         MINUS          =   45, 
         PERIOD         =   46, 
         SLASH          =   47, 
         NUM0              =   48,
         NUM1              =   49,
         NUM2              =   50,
         NUM3              =   51,
         NUM4              =   52,
         NUM5              =   53,
         NUM6              =   54,
         NUM7              =   55,
         NUM8              =   56,
         NUM9              =   57,
         SEMICOLON      =   59, 
         EQUAL          =   61, 
         A              =   65,
         B              =   66,
         C              =   67,
         D              =   68,
         E              =   69,
         F              =   70,
         G              =   71,
         H              =   72,
         I              =   73,
         J              =   74,
         K              =   75,
         L              =   76,
         M              =   77,
         N              =   78,
         O              =   79,
         P              =   80,
         Q              =   81,
         R              =   82,
         S              =   83,
         T              =   84,
         U              =   85,
         V              =   86,
         W              =   87,
         X              =   88,
         Y              =   89,
         Z              =   90,
         LEFT_BRACKET   =   91, 
         BACKSLASH      =   92, 
         RIGHT_BRACKET  =   93, 
         GRAVE_ACCENT   =   96, 
         WORLD_1        =   161,
         WORLD_2        =   162,
         ESCAPE         =   256,
         ENTER          =   257,
         TAB            =   258,
         BACKSPACE      =   259,
         INSERT         =   260,
         DELETE         =   261,
         RIGHT          =   262,
         LEFT           =   263,
         DOWN           =   264,
         UP             =   265,
         PAGE_UP        =   266,
         PAGE_DOWN      =   267,
         HOME           =   268,
         END            =   269,
         CAPS_LOCK      =   280,
         SCROLL_LOCK    =   281,
         NUM_LOCK       =   282,
         PRINT_SCREEN   =   283,
         PAUSE          =   284,
         F1             =   290,
         F2             =   291,
         F3             =   292,
         F4             =   293,
         F5             =   294,
         F6             =   295,
         F7             =   296,
         F8             =   297,
         F9             =   298,
         F10            =   299,
         F11            =   300,
         F12            =   301,
         F13            =   302,
         F14            =   303,
         F15            =   304,
         F16            =   305,
         F17            =   306,
         F18            =   307,
         F19            =   308,
         F20            =   309,
         F21            =   310,
         F22            =   311,
         F23            =   312,
         F24            =   313,
         F25            =   314,
         KP_0           =   320,
         KP_1           =   321,
         KP_2           =   322,
         KP_3           =   323,
         KP_4           =   324,
         KP_5           =   325,
         KP_6           =   326,
         KP_7           =   327,
         KP_8           =   328,
         KP_9           =   329,
         KP_DECIMAL     =   330,
         KP_DIVIDE      =   331,
         KP_MULTIPLY    =   332,
         KP_SUBTRACT    =   333,
         KP_ADD         =   334,
         KP_ENTER       =   335,
         KP_EQUAL       =   336,
         LEFT_SHIFT     =   340,
         LEFT_CONTROL   =   341,
         LEFT_ALT       =   342,
         LEFT_SUPER     =   343,
         RIGHT_SHIFT    =   344,
         RIGHT_CONTROL  =   345,
         RIGHT_ALT      =   346,
         RIGHT_SUPER    =   347,
         MENU           =   348,
    }

    public class Input
    {
        internal static unsafe delegate*<int, bool> IsKeyDownIcall;
        internal static unsafe delegate*<NativeArray<float>> GetMousePositionIcall;

        public static bool IsKeyDown(Key keys)
        {
            unsafe { return IsKeyDownIcall((int)keys); }
        }

        public static Vector2 GetMousePosition()
        {
            NativeArray<float> result;
            unsafe { result = GetMousePositionIcall(); }

            return new Vector2(result[0], result[1]);
        }
    }

    public class Scene
    {
        internal static unsafe delegate*<NativeString, UInt32> GetEntityIcall;

        public static Entity GetEntity(string entityName)
        {

            uint handle;
            unsafe { handle = GetEntityIcall(entityName); }

            if(handle == UInt32.MaxValue)
            {
                throw new Exception("Entity not found");
            }

            Entity entity = new Entity();
            entity.ECSHandle = handle;
            return entity;
        }
    }

    public class IComponent
    {
        public UInt32 EntityID { get; protected set; }
    }
    public class ParentComponent : IComponent
    {
        public ParentComponent(UInt32 entityId) { }

        public Entity Parent { get; set; }
        public List<Entity> Children { get; set; }
    }
    public class NameComponent : IComponent
    {
        public NameComponent(UInt32 entityId)
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

    public class AudioEmitterComponent : IComponent
    {
        public AudioEmitterComponent(UInt32 entityId)
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
        public ModelComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public string Mesh { get; set; }
    }

    public class SkinnedModelComponent : IComponent
    {
        public SkinnedModelComponent(UInt32 entityId) { }

        public bool Playing { get; set; }
        public int CurrentAnimation { get; set; }
    }

    public class BoneComponent : IComponent
    {
        public BoneComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }
        public string Name { get; set; }

    }
    public class RigidbodyComponent : IComponent
    {
        public RigidbodyComponent(UInt32 entityId) { }

        public float Mass { get; set; }
    }

    public class BoxCollider : IComponent
    {
        public BoxCollider(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public Vector3 Scale { get; set; }
    }

    public class SphereCollider : IComponent
    {
        public SphereCollider(UInt32 entityId)
        {
            EntityID = entityId;
        }

        public float Radius { get; set; }
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

    public class ParticleEmitterComponent : IComponent
    {
        public ParticleEmitterComponent(UInt32 entityId) { }

        public float Amount { get; set; }
        public float Life {  get; set; }
        public Vector3 Gravity { get; set; }
    }
    public class QuakeMapComponent : IComponent
    {
        public QuakeMapComponent(UInt32 entityId) { }

        public string Map { get; set; }

        public bool BuildCollisions { get; set; }

        public void Build() { }
    }
    public class BSPBrushComponent : IComponent
    {
        public BSPBrushComponent(UInt32 entityId)
        {
            EntityID = entityId;
        }
    }
    public class SpriteComponent : IComponent
    {
        public SpriteComponent(UInt32 entityId) { }

        public string Sprite { get; set; }
    }


    public class Entity
    {
        internal static unsafe delegate*<UInt32, UInt32, bool> EntityHasComponentIcall;

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

        public UInt32 ID { get; private set; }
        public UInt32 ECSHandle { get; set; }
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

        public bool HasComponent<T>() where T : IComponent
        {
            if(MappingTypeEnum.ContainsKey(typeof(T)))
            {
                unsafe { return EntityHasComponentIcall(ID, (UInt32)ComponentTypes.TRANSFORM); };
            }

            return false;
        }

        public T? GetComponent<T>() where T : IComponent
        {
            if(HasComponent<T>())
            {
                return (T)Activator.CreateInstance(typeof(T), ID);
            }

            return null;
        }
    }

}