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
				else if (dataType.type() == entt::resolve<std::string>())
				{
					std::string value = fieldVal.cast<std::string>();
					cursor[displayName] = value;
				}
				else if (auto prop = dataType.prop(HashedFieldPropName::IsEnum); prop)
				{
					auto enumMeta = dataType.type();
					// Fallback to integer value if name not available
					cursor[displayName] = static_cast<int>(fieldVal.cast<int>());
				}
			}

			return jsonSnippet;
		}

		template<IsComponentT T>
		void Deserialize(const json& jsonSnippet, T& component)
		{
			const entt::meta_type meta = entt::resolve<T>();
			if (!meta) return;

			entt::meta_any metaAny = meta.from_void(static_cast<void*>(&component));
			if (!metaAny) return;

			const std::string componentName = Component::GetName(meta);
			if (!jsonSnippet.contains(componentName)) return;

			const json& cursor = jsonSnippet.at(componentName);

			for (auto [fst, dataMember] : meta.data())
			{
				auto propName = dataMember.prop(HashedName::DisplayName);
				if (!propName) continue;

				const char* displayNameC = *propName.value().try_cast<const char*>();
				std::string displayName = displayNameC;

				if (!cursor.contains(displayName)) continue;

				const entt::meta_type fieldType = dataMember.type();

				if (fieldType == entt::resolve<float>())
				{
					dataMember.set(metaAny, cursor.at(displayName).get<float>());
				}
				else if (fieldType == entt::resolve<int32_t>())
				{
					dataMember.set(metaAny, cursor.at(displayName).get<int32_t>());
				}
				else if (fieldType == entt::resolve<bool>())
				{
					dataMember.set(metaAny, cursor.at(displayName).get<bool>());
				}
				else if (fieldType == entt::resolve<Vector2>())
				{
					Vector2 vec;
					vec.x = cursor.at(displayName).at("x").get<float>();
					vec.y = cursor.at(displayName).at("y").get<float>();
					dataMember.set(metaAny, vec);
				}
				else if (fieldType == entt::resolve<Vector3>())
				{
					Vector3 vec;
					vec.x = cursor.at(displayName).at("x").get<float>();
					vec.y = cursor.at(displayName).at("y").get<float>();
					vec.z = cursor.at(displayName).at("z").get<float>();
					dataMember.set(metaAny, vec);
				}
				else if (fieldType == entt::resolve<Vector4>())
				{
					Vector4 vec;
					vec.x = cursor.at(displayName).at("x").get<float>();
					vec.y = cursor.at(displayName).at("y").get<float>();
					vec.z = cursor.at(displayName).at("z").get<float>();
					vec.w = cursor.at(displayName).at("w").get<float>();
					dataMember.set(metaAny, vec);
				}
				else if (fieldType == entt::resolve<ResourceFile>())
				{
					std::string fileKey = "file" + displayName;
					if (cursor.contains(fileKey))
					{
						//std::string relativePath = cursor.at(fileKey).get<std::string>();
						//ResourceFile resource;
						//resource.file = std::make_shared<File>(relativePath);
						//dataMember.set(metaAny, resource);
					}
				}
				else if (fieldType == entt::resolve<std::string>())
				{
					dataMember.set(metaAny, cursor.at(displayName).get<std::string>());
				}
			}
		}
	};
}