#pragma once
#include "Nuake/Core/Object/Object.h"

#include <entt/entt.hpp>
#include <imgui/imgui.h>

#include <functional>
#include <unordered_map>


namespace Nuake
{
	using DrawWidgetTypeFn = std::function<void(entt::meta_data& fieldMeta, entt::meta_any& instance)>;
	class WidgetDrawer
	{
	public:
		WidgetDrawer() = default;
		~WidgetDrawer() = default;

		static WidgetDrawer& Get()
		{
			static WidgetDrawer instance;
			return instance;
		}

		void DrawFloat(entt::meta_data& type, entt::meta_any& instance)
		{
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

		void DrawWidget(entt::meta_data& dataType, entt::meta_any& instance)
		{
			entt::id_type dataId = dataType.type().id();
			if (WidgetTypeDrawers.contains(dataId))
			{
				auto& drawerFn = WidgetTypeDrawers[dataId];
				drawerFn(dataType, instance);
			}
			else
			{
				ImGui::Text("ERR");
			}
		}

		template<class T, auto Func, class O>
		void RegisterTypeDrawer(O* o)
		{
			WidgetTypeDrawers[entt::type_id<T>().hash()] = std::bind(Func, o, std::placeholders::_1, std::placeholders::_2);
		}

	private:
		std::unordered_map<entt::id_type, DrawWidgetTypeFn> WidgetTypeDrawers;
	};
}