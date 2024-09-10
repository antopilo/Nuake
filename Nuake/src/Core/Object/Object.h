#pragma once

#include <entt/entt.hpp>

#define NK_HASHED_FN_NAME_IMPL(name) inline static constexpr entt::hashed_string name = entt::hashed_string(#name);

namespace Nuake
{
    struct HashedFnName
    {
        NK_HASHED_FN_NAME_IMPL(GetComponentName)
        NK_HASHED_FN_NAME_IMPL(AddToEntity)
    };
}

#define NUAKECOMPONENT(klass, componentName)                                      \
public:                                                                           \
    static std::string ClassName()                                                \
    {                                                                             \
        static std::string className = #klass;                                    \
        return className;                                                         \
    }                                                                             \
                                                                                  \
    static std::string ComponentName()                                            \
    {                                                                             \
        static std::string name = componentName;                                  \
        return name;                                                              \
    }                                                                             \
                                                                                  \
    static void AddToEntity(entt::entity entity, entt::registry* enttRegistry)    \
    {                                                                             \
        enttRegistry->emplace_or_replace<klass>(entity);                          \
    }                                                                             \
                                                                                  \
    inline static auto ComponentFactory = entt::meta<klass>()                     \
        .type(entt::hashed_string(#klass))                                        \
        .func<&klass::ComponentName>(HashedFnName::GetComponentName)              \
        .func<&klass::AddToEntity>(HashedFnName::AddToEntity);
