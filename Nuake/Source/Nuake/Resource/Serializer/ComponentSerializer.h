#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Scene/Components/Component.h"
#include "Nuake/Resource/Serializable.h"
#include "Nuake/FileSystem/File.h"

#include <concepts>

namespace Nuake
{
	template<typename T>
	concept IsComponentT = std::derived_from<T, Component>;

	// This will output JSON data from any component using the reflection data
	class ComponentSerializer
	{
	public:
		ComponentSerializer() = default;
		~ComponentSerializer() = default;

	public:
		template<IsComponentT T>
		json Serialize(T& component)
		{
			json jsonSnippet;

			const entt::meta_type& meta = entt::resolve<T>();
			const entt::meta_any& metaAny = meta.from_void(static_cast<void*>(&component));
			const std::string componentName = Component::GetName(meta);

			json& cursor = jsonSnippet[componentName];
			for (auto [fst, dataType] : meta.data())
			{
				const ComponentFieldTrait fieldTraits = dataType.traits<ComponentFieldTrait>();
				entt::id_type dataId = dataType.type().id();

				auto propName = dataType.prop(HashedName::DisplayName);
				const char* displayNameC = *propName.value().try_cast<const char*>();
				std::string displayName = displayNameC;
				auto fieldVal = dataType.get(metaAny);
				if (dataType.type() == entt::resolve<float>())
				{
					float value = fieldVal.cast<float>();
					cursor[displayName] = value;
				}
				else if(dataType.type() == entt::resolve<int32_t>())
				{
					int32_t value = fieldVal.cast<int32_t>();
					cursor[displayName] = value;
				}
				else if (dataType.type() == entt::resolve<bool>())
				{
					bool value = fieldVal.cast<bool>();
					cursor[displayName] = value;
				}
				else if (dataType.type() == entt::resolve<Vector2>())
				{
					Vector2 value = fieldVal.cast<Vector2>();
					cursor[displayName]["x"] = value.x;
					cursor[displayName]["y"] = value.y;
				}
				else if (dataType.type() == entt::resolve<Vector3>())
				{
					Vector3 value = fieldVal.cast<Vector3>();
					cursor[displayName]["x"] = value.x;
					cursor[displayName]["y"] = value.y;
					cursor[displayName]["z"] = value.z;
				}
				else if (dataType.type() == entt::resolve<Vector4>())
				{
					Vector4 value = fieldVal.cast<Vector4>();
					cursor[displayName]["x"] = value.x;
					cursor[displayName]["y"] = value.y;
					cursor[displayName]["z"] = value.z;
					cursor[displayName]["w"] = value.w;
				}
				else if (dataType.type() == entt::resolve<ResourceFile>())
				{
					ResourceFile value = fieldVal.cast<ResourceFile>();
					bool validFile = value.file != nullptr && value.file->Exist();
					cursor["validFile" + displayName] = validFile;
					if (validFile)
					{ 
						cursor["file" + displayName] = value.file->GetRelativePath();
					}
				}
			}

			return jsonSnippet;
		}
	};
}