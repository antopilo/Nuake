#pragma once
#include "Nuake/Scene/Components/Component.h"
#include "Nuake/Resource/Serializable.h"

namespace Nuake
{
	// This will output JSON data from any component using the reflection data
	class ComponentSerializer
	{
	public:
		ComponentSerializer() = default;
		~ComponentSerializer() = default;

	public:
		template<typename T>
		json Serialize(const T& component)
		{
			json jsonSnippet;

			const entt::meta_type& meta = entt::resolve<T>();
			const std::string componentName = Component::GetName(meta);

			json cursor = j[componentName];
			for (auto [fst, dataType] : meta.data())
			{
				const std::string fieldName = fst.name();
				const entt::meta_any value = meta.data(fieldName).get(component);
				if (value.type() == entt::resolve<bool>())
				{
					cursor[fieldName] = value.cast<bool>();
				}
				else if (value.type() == entt::resolve<std::string>())
				{
					cursor[fieldName] = value.cast<std::string>();
				}
			}

			return jsonSnippet;
		}
	};
}