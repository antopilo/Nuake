#include "Component.h"

using namespace Nuake;

std::string Component::GetName(const entt::meta_type& componentMeta)
{
    // TODO: [WiggleWizard] Needs some error handling
    if (entt::meta_func func = componentMeta.func(HashedFnName::GetComponentName))
    {
        entt::meta_any ret = func.invoke(componentMeta);
        return ret.cast<std::string>();
    }

    return "Unknown Component";
}
