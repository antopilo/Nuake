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
		WidgetDrawer()
		{
			RegisterDefaulWidgets();
		}

		~WidgetDrawer() = default;

		static WidgetDrawer& Get()
		{
			static WidgetDrawer instance;
			return instance;
		}

		void RegisterDefaulWidgets()
		{
			auto& drawer = Get();
			drawer.RegisterTypeDrawer<float, &WidgetDrawer::DrawFloat>(&drawer);
			drawer.RegisterTypeDrawer<bool, &WidgetDrawer::DrawBool>(&drawer);
			drawer.RegisterTypeDrawer<std::string, &WidgetDrawer::DrawString>(&drawer);
			drawer.RegisterTypeDrawer<Vector2, &WidgetDrawer::DrawVector2>(&drawer);
			drawer.RegisterTypeDrawer<Vector3, &WidgetDrawer::DrawVector3>(&drawer);
			drawer.RegisterTypeDrawer<DynamicItemList, &WidgetDrawer::DrawDynamicItemList>(&drawer);
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

		void DrawBool(entt::meta_data& type, entt::meta_any& instance)
		{
			auto prop = type.prop(HashedName::DisplayName);
			auto propVal = prop.value();
			const char* displayName = *propVal.try_cast<const char*>();

			if (displayName != nullptr)
			{
				ImGui::Text(displayName);
				ImGui::TableNextColumn();

				auto fieldVal = type.get(instance);
				bool* boolPtr = fieldVal.try_cast<bool>();
				if (boolPtr != nullptr)
				{
					bool boolProxy = *boolPtr;
					std::string controlId = std::string("##") + displayName;
					if (ImGui::Checkbox(controlId.c_str(), &boolProxy))
					{
						type.set(instance, boolProxy);
					}
				}
				else
				{
					ImGui::Text("ERR");
				}
			}
		}

		void DrawString(entt::meta_data& type, entt::meta_any& instance)
		{
			auto prop = type.prop(HashedName::DisplayName);
			auto propVal = prop.value();
			const char* displayName = *propVal.try_cast<const char*>();

			if (displayName != nullptr)
			{
				ImGui::Text(displayName);
				ImGui::TableNextColumn();

				auto fieldVal = type.get(instance);
				std::string* fieldValPtr = fieldVal.try_cast<std::string>();
				if (fieldValPtr != nullptr)
				{
					std::string fieldValProxy = *fieldValPtr;
					std::string controlId = std::string("##") + displayName;
					ImGui::InputText(controlId.c_str(), &fieldValProxy);

					if (fieldValProxy != *fieldValPtr)
					{
						type.set(instance, fieldValProxy);
					}
				}
				else
				{
					ImGui::Text("ERR");
				}
			}
		}

		void DrawVector2(entt::meta_data& field, entt::meta_any& component)
		{
			auto prop = field.prop(HashedName::DisplayName);
			auto propVal = prop.value();
			const char* displayName = *propVal.try_cast<const char*>();

			if (displayName != nullptr)
			{
				ImGui::Text(displayName);
				ImGui::TableNextColumn();

				auto fieldVal = field.get(component);
				Vector2* vec2Ptr = fieldVal.try_cast<Vector2>();
				std::string controlId = std::string("##") + displayName;
				ImGui::PushID(controlId.c_str());

				if (ImGuiHelper::DrawVec2(controlId, vec2Ptr, 0.5f, 100.0, 0.01f))
				{
					field.set(component, *vec2Ptr);
					Engine::GetProject()->IsDirty = true;
				}

				ImGui::PopID();
			}
		}

		void DrawVector3(entt::meta_data& field, entt::meta_any& component)
		{
			auto prop = field.prop(HashedName::DisplayName);
			auto propVal = prop.value();
			const char* displayName = *propVal.try_cast<const char*>();

			if (displayName != nullptr)
			{
				ImGui::Text(displayName);
				ImGui::TableNextColumn();

				auto fieldVal = field.get(component);
				Vector3* vec3Ptr = fieldVal.try_cast<Vector3>();
				std::string controlId = std::string("##") + displayName;
				ImGui::PushID(controlId.c_str());

				if (ImGuiHelper::DrawVec3(controlId, vec3Ptr, 0.5f, 100.0, 0.01f))
				{
					field.set(component, *vec3Ptr);
					Engine::GetProject()->IsDirty = true;
				}

				ImGui::PopID();
			}
		}

		void DrawDynamicItemList(entt::meta_data& field, entt::meta_any& component)
		{
			auto propDisplayName = field.prop(HashedName::DisplayName);
			const char* displayName = *propDisplayName.value().try_cast<const char*>();
			if (displayName != nullptr)
			{
				ImGui::Text(displayName);
				ImGui::TableNextColumn();

				auto fieldVal = field.get(component);
				auto fieldValPtr = fieldVal.try_cast<DynamicItemList>();
				if (fieldValPtr == nullptr)
				{
					ImGui::Text("ERR");
				}

				const auto& items = fieldValPtr->items;
				const int index = fieldValPtr->index;

				// Check first to see if we are within the bounds
				std::string selectedStr = "";
				if (index >= 0 || index < items.size())
				{
					selectedStr = items[index];
				}

				std::string controlName = std::string("##") + displayName;
				if (ImGui::BeginCombo(controlName.c_str(), selectedStr.c_str()))
				{
					for (int i = 0; i < items.size(); i++)
					{
						bool isSelected = (index == i);
						std::string name = items[i];

						if (name.empty())
						{
							name = "Empty";
						}

						if (ImGui::Selectable(name.c_str(), isSelected))
						{
							field.set(component, i);
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
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