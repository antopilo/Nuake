#pragma once

#include <entt/entt.hpp>
#include <type_traits>

#define NK_HASHED_STATIC_STR(name) inline static constexpr entt::hashed_string name = entt::hashed_string(#name);

namespace Nuake
{
    struct HashedFnName
    {
        NK_HASHED_STATIC_STR(GetComponentName)
        NK_HASHED_STATIC_STR(AddToEntity)
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
    };

    enum class ComponentTypeTrait : uint16_t
    {
        None = 0,
        InspectorExposed = 1 << 0,

        
    };

    template <typename Enum>
    constexpr std::underlying_type_t<Enum> ToUnderlying(Enum e)
    {
        return static_cast<std::underlying_type_t<Enum>>(e);
    }

    inline ComponentTypeTrait operator|(ComponentTypeTrait lhs, ComponentTypeTrait rhs)
    {
        return static_cast<ComponentTypeTrait>(ToUnderlying(lhs) | ToUnderlying(rhs));
    }

    inline ComponentTypeTrait operator&(ComponentTypeTrait lhs, ComponentTypeTrait rhs)
    {
        return static_cast<ComponentTypeTrait>(ToUnderlying(lhs) & ToUnderlying(rhs));
    }
}

#define NUAKECOMPONENT(klass, componentName)                                                                                                                                             \
public:                                                                                                                                                                                  \
    static std::string ClassName()                                                                \
    {                                                                                             \
        static std::string className = #klass;                                                    \
        return className;                                                                         \
    }                                                                                             \
                                                                                                  \
    static std::string ComponentName()                                                            \
    {                                                                                             \
        static std::string name = componentName;                                                  \
        return name;                                                                              \
    }                                                                                             \
                                                                                                  \
    static void AddToEntity(entt::entity entity, entt::registry* enttRegistry)                    \
    {                                                                                             \
        enttRegistry->emplace_or_replace<klass>(entity);                                          \
    }                                                                                             \
                                                                                                  \
                                                                                                  \
    static void (*GetInitializeComponentClass())()                                                \
    {                                                                                             \
        return &klass::InitializeComponentClass;                                                  \
    }                                                                                             \
                                                                                                  \
    inline static auto ComponentFactory = entt::meta<klass>();                                    \
    static void InternalInitializeClass()                                                         \
    {                                                                                             \
        static bool initialized = false;                                                          \
        if (initialized)                                                                          \
            return;                                                                               \
                                                                                                  \
        ComponentFactory.type(entt::hashed_string(#klass))                                        \
            .traits(ComponentTypeTrait::InspectorExposed);                                        \
        ComponentFactory.func<&klass::ComponentName>(HashedFnName::GetComponentName);             \
        ComponentFactory.func<&klass::AddToEntity>(HashedFnName::AddToEntity);                    \
                                                                                                  \
        if (klass::GetInitializeComponentClass() != Component::GetInitializeComponentClass())     \
        {                                                                                         \
            GetInitializeComponentClass()();                                                      \
        }                                                                                         \
                                                                                                  \
        initialized = true;                                                                       \
                                                                                                  \
    }                                                                                             \
                                                                                                  \
    template<auto Data>                                                                           \
    static auto BindComponentField(const char* varName, const char* displayName)                  \
    {                                                                                             \
        return ComponentFactory                                                                   \
            .data<Data>(entt::hashed_string(varName))                                             \
            .prop(HashedName::DisplayName, displayName);                                          \
    }                                                                                             \
                                                                                                  \
    template<auto Getter, auto Setter>                                                            \
    static auto BindComponentProperty(const char* varName, const char* displayName)               \
    {                                                                                             \
        return ComponentFactory                                                                   \
            .data<Getter, Setter>(entt::hashed_string(varName))                                   \
            .prop(HashedName::DisplayName, displayName);                                          \
    }                                                                                             \
                                                                                                  \
    static auto FloatFieldLimits(float stepSize, float min, float max)                            \
    {                                                                                             \
        return ComponentFactory                                                                   \
            .prop(HashedFieldPropName::FloatStep, stepSize)                                       \
            .prop(HashedFieldPropName::FloatMin, min)                                             \
            .prop(HashedFieldPropName::FloatMax, max);                                            \
    }                                                                                             \
    \
    static auto ResourceFileRestriction(const char* fileType)  \
    {  \
        return ComponentFactory  \
            .prop(HashedFieldPropName::ResourceFileType, fileType);  \
    }
