// Auto-generated modules management file
#include "Modules.h"

#include "AssimpModule/AssimpModule.h"
#include "AudioModule/AudioModule.h"
#include "ExampleModule/ExampleModule.h"

#include "Nuake/UI/WidgetDrawer.h"

#include "Nuake/Core/Object/Object.h"
#include "Nuake/Core/Logger.h"

void DrawFloatWidget(entt::meta_data& type, entt::meta_any& instance)
{
    using namespace Nuake;

    float stepSize = 1.f;
    if (auto prop = type.prop(HashedFieldPropName::FloatStep))
        stepSize = *prop.value().try_cast<float>();

    float min = 0.f;
    if (auto prop = type.prop(HashedFieldPropName::FloatMin))
        min = *prop.value().try_cast<float>();

    float max = 0.f;
    if (auto prop = type.prop(HashedFieldPropName::FloatMax))
        max = *prop.value().try_cast<float>();

    auto propDisplayName = type.prop(HashedName::DisplayName);
    const char* displayName = *propDisplayName.value().try_cast<const char*>();
    if (displayName != nullptr)
    {
        ImGui::Text(displayName);
        ImGui::TableNextColumn();

        auto fieldVal = type.get(instance);
        float* floatPtr = fieldVal.try_cast<float>();
        if (floatPtr != nullptr)
        {
            float floatProxy = *floatPtr;
            const std::string controlId = std::string("##") + displayName;
            if (ImGui::DragFloat(controlId.c_str(), &floatProxy, stepSize, min, max))
            {
                type.set(instance, floatProxy);
            }
        }
        else
        {
            ImGui::Text("ERR");
        }
    }
}

void DrawBoolWidget(entt::meta_type& type, entt::meta_any& instance)
{

}

void Nuake::Modules::StartupModules()
{
    auto& drawer = WidgetDrawer::Get();
    drawer.RegisterTypeDrawer<float, &WidgetDrawer::DrawFloat>(&drawer);

    //drawer.RegisterTypeDrawer<bool, DrawBoolWidget>(&DrawBoolWidget);

    Logger::Log("Starting AssimpModule", "modules");
    AssimpModule_Startup();
    Logger::Log("Starting AudioModule", "modules");
    AudioModule_Startup();
    Logger::Log("Starting ExampleModule", "modules");
    ExampleModule_Startup();
}

void Nuake::Modules::ShutdownModules()
{
    Logger::Log("Shutting down AssimpModule", "modules");
    AssimpModule_Shutdown();
    Logger::Log("Shutting down AudioModule", "modules");
    AudioModule_Shutdown();
    Logger::Log("Shutting down ExampleModule", "modules");
    ExampleModule_Shutdown();
}
