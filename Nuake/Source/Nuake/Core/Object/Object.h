#pragma once

#include <entt/entt.hpp>
#include <type_traits>

#define NK_HASHED_STATIC_STR(name) inline static constexpr entt::hashed_string name = entt::hashed_string(#name);

#define NK_ENUM_BITWISE_IMPL(enumClassName) \
    inline enumClassName operator|(enumClassName lhs, enumClassName rhs) \
    { \
        return static_cast<enumClassName>(ToUnderlying(lhs) | ToUnderlying(rhs)); \
    } \
 \
    inline enumClassName operator&(enumClassName lhs, enumClassName rhs) \
    { \
        return static_cast<enumClassName>(ToUnderlying(lhs) & ToUnderlying(rhs)); \
    }

namespace Nuake
{
    struct HashedFnName
    {
        NK_HASHED_STATIC_STR(GetComponentName)
        NK_HASHED_STATIC_STR(AddToEntity)
        NK_HASHED_STATIC_STR(RemoveFromEntity)
        NK_HASHED_STATIC_STR(ActionName)
        NK_HASHED_STATIC_STR(ModuleSettingName)
    };

    struct HashedFieldPropName
    {
        NK_HASHED_STATIC_STR(FloatStep)
        NK_HASHED_STATIC_STR(FloatMin)
        NK_HASHED_STATIC_STR(FloatMax)

        NK_HASHED_STATIC_STR(ResourceFileType)
    };

    struct HashedName
    {
        NK_HASHED_STATIC_STR(DisplayName)
        NK_HASHED_STATIC_STR(ArgsName)
    };

    struct HashedUserValue
    {
        NK_HASHED_STATIC_STR(FuncPtr)
    };

    enum class ComponentTypeTrait : uint16_t
    {
        None = 0,
        // Exposes the component to be added via the inspector
        InspectorExposed = 1 << 0,
    };

    enum class ComponentFuncTrait : uint16_t
    {
        None = 0,
        // Exposes the component to be added via the inspector
        Action = 1 << 0,
    };

    enum class ComponentFieldTrait : uint16_t
    {
        None = 0,
        // Stops field from showing up in the editor inspector
        Internal = 1 << 0,
        // Marks the field as temporary (ie, do not serialize)
        Transient = 1 << 1,
    };

    template <typename Enum>
    constexpr std::underlying_type_t<Enum> ToUnderlying(Enum e)
    {
        return static_cast<std::underlying_type_t<Enum>>(e);
    }

    NK_ENUM_BITWISE_IMPL(ComponentTypeTrait);
    NK_ENUM_BITWISE_IMPL(ComponentFuncTrait);
    NK_ENUM_BITWISE_IMPL(ComponentFieldTrait);
}

#define NUAKECOMPONENT(klass, componentName)                                                                                                                                             \
public:                                                                                                                                                                                  \
    static std::string ClassName()                                                                    \
    {                                                                                                 \
        static std::string className = #klass;                                                        \
        return className;                                                                             \
    }                                                                                                 \
                                                                                                      \
    static std::string ComponentName()                                                                \
    {                                                                                                 \
        static std::string name = componentName;                                                      \
        return name;                                                                                  \
    }                                                                                                 \
                                                                                                      \
    static void AddToEntity(entt::entity entity, entt::registry* enttRegistry)                        \
    {                                                                                                 \
        enttRegistry->emplace_or_replace<klass>(entity);                                              \
    }                                                                                                 \
                                                                                                      \
	static void RemoveFromEntity(entt::entity entity, entt::registry* enttRegistry)                   \
	{                                                                                                 \
		enttRegistry->erase<klass>(entity);                                                           \
	}                                                                                                 \
                                                                                                      \
                                                                                                      \
    static void (*GetInitializeComponentClass())()                                                    \
    {                                                                                                 \
        return &klass::InitializeComponentClass;                                                      \
    }                                                                                                 \
                                                                                                      \
    inline static auto ComponentFactory = entt::meta<klass>();                                        \
    static void InternalInitializeClass()                                                             \
    {                                                                                                 \
        static bool initialized = false;                                                              \
        if (initialized)                                                                              \
            return;                                                                                   \
                                                                                                      \
        ComponentFactory.type(entt::hashed_string(#klass))                                            \
            .traits(ComponentTypeTrait::InspectorExposed);                                            \
        ComponentFactory.func<&klass::ComponentName>(HashedFnName::GetComponentName);                 \
        ComponentFactory.func<&klass::AddToEntity>(HashedFnName::AddToEntity);                        \
        ComponentFactory.func<&klass::RemoveFromEntity>(HashedFnName::RemoveFromEntity);              \
                                                                                                      \
        if (klass::GetInitializeComponentClass() != Component::GetInitializeComponentClass())         \
        {                                                                                             \
            GetInitializeComponentClass()();                                                          \
        }                                                                                             \
                                                                                                      \
        initialized = true;                                                                           \
                                                                                                      \
    }                                                                                                 \
                                                                                                      \
    template<auto Data>                                                                               \
    static auto BindComponentField(const char* varName, const char* displayName)                      \
    {                                                                                                 \
        return ComponentFactory                                                                       \
            .data<Data>(entt::hashed_string(varName))                                                 \
            .prop(HashedName::DisplayName, displayName);                                              \
    }                                                                                                 \
                                                                                                      \
    template<auto Getter, auto Setter>                                                                \
    static auto BindComponentProperty(const char* varName, const char* displayName)                   \
    {                                                                                                 \
        return ComponentFactory                                                                       \
            .data<Getter, Setter>(entt::hashed_string(varName))                                       \
            .prop(HashedName::DisplayName, displayName);                                              \
    }                                                                                                 \
                                                                                                      \
    static auto FieldFloatLimits(float stepSize, float min, float max)                                \
    {                                                                                                 \
        return ComponentFactory                                                                       \
            .prop(HashedFieldPropName::FloatStep, stepSize)                                           \
            .prop(HashedFieldPropName::FloatMin, min)                                                 \
            .prop(HashedFieldPropName::FloatMax, max);                                                \
    }                                                                                                 \
                                                                                                      \
    static auto ResourceFileRestriction(const char* fileType)                                         \
    {                                                                                                 \
        return ComponentFactory                                                                       \
            .prop(HashedFieldPropName::ResourceFileType, fileType);                                   \
    }                                                                                                 \
                                                                                                      \
    template <typename... Enums>                                                                      \
    static auto SetFlags(Enums... enums)                                                              \
    {                                                                                                 \
        static_assert((std::is_enum_v<Enums> && ...), "All arguments must be of enum class type");    \
        return ComponentFactory.traits((enums | ...));                                                \
    }                                                                                                 \
                                                                                                      \
    template<auto Func>                                                                               \
    static void BindAction(const char* funcName, const char* actionName)                              \
    {                                                                                                 \
        ComponentFactory                                                                              \
            .func<Func>(entt::hashed_string(funcName))                                                \
            .prop(HashedName::DisplayName, actionName);                                               \
        SetFlags(ComponentFuncTrait::Action);                                                         \
    }
