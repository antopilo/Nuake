#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include "src/Rendering/Shaders/Shader.h"
#include "src/Rendering/Textures/TextureManager.h"
#include "Texture.h"
#include <src/Resource/Serializable.h>
#include "src/Resource/Resource.h"
#include "src/FileSystem/FileSystem.h"

#include "src/Rendering/Vulkan/VkResources.h"
#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"


namespace Nuake
{
	struct UBOStructure {
		int u_HasAlbedo;
		float padding;
		float padding2;
		float padding3;
		Vector3 m_AlbedoColor;
		int u_HasMetalness;
		float u_MetalnessValue;
		int u_HasRoughness;
		float u_RoughnessValue;
		int u_HasAO;
		float u_AOValue;
		int u_HasNormal;
		int u_HasDisplacement;
		int u_Unlit;
		float u_Emissive;
	};

	class Material : ISerializable, public Resource
	{
	private:
		std::string m_Name;
		unsigned int UBO;

		void InitUniformBuffer()
		{
			data = {
				0, // Has albedo texture
				0, // Padding byte
				0, // Padding byte
				0, // Padding byte
				Vector3(1.f, 1.f, 1.f),	// Albedo color
				0,						// Has metalness 
				0.f,					// Metalness value
				0,						// u_HasRoughness
				1.f,					// u_RoughnessValue
				0,						// u_HasAO
				0.5f,					// u_AOValue
				0,						// u_HasNormal
				0,						// u_HasDisplacement
				0, // unlit
				1.0f // emissive
			};
		}
	public:
		UUID AlbedoImage = UUID(0);
		UUID AOImage;
		UUID MetalnessImage;
		UUID RoughnessImage;
		UUID NormalImage;
		
		Ref<Texture> m_Albedo;
		Ref<Texture> m_AO;
		Ref<Texture> m_Metalness;
		Ref<Texture> m_Roughness;
		Ref<Texture> m_Normal;
		Ref<Texture> m_Displacement;

		UBOStructure data;

		static Ref<Texture> m_DefaultAlbedo;
		static Ref<Texture> m_DefaultAO;
		static Ref<Texture> m_DefaultMetalness;
		static Ref<Texture> m_DefaultRoughness;
		static Ref<Texture> m_DefaultNormal;
		static Ref<Texture> m_DefaultDisplacement;

		Material();
		Material(const std::string albedo);
		Material(Ref<Texture> texture) { m_Albedo = texture; }
		Material(const Vector3 albedoColor);
		~Material();

		void InitDefaultTextures();

		void Bind(Shader* shader);
		void SetupUniformBuffer();

		void SetName(const std::string name);
		std::string GetName();

		inline void SetUnlit(bool value) { data.u_Unlit = value; }
		inline bool GetUnlit() { return data.u_Unlit == 1; }

		bool HasAlbedo() { return (m_Albedo != nullptr || AlbedoImage != UUID(0)); }
		void SetAlbedo(const std::string path) { m_Albedo = CreateRef<Texture>(path); }
		void SetAlbedo(Ref<Texture> texture) { m_Albedo = texture; }

		bool HasAO() { return m_AO != nullptr; }
		void SetAO(const std::string albedo);
		void SetAO(Ref<Texture> texture) { m_AO = texture; }

		bool HasMetalness() { return m_Metalness != nullptr; }
		void SetMetalness(const std::string albedo);
		void SetMetalness(Ref<Texture> texture) { m_Metalness = texture; }

		bool HasRoughness() { return m_Roughness != nullptr; }
		void SetRoughness(const std::string albedo);
		void SetRoughness(Ref<Texture> texture) { m_Roughness = texture; }

		bool HasNormal() { return m_Normal != nullptr; }
		void SetNormal(const std::string albedo);
		void SetNormal(Ref<Texture> texture) { m_Normal = texture; }

		bool HasDisplacement() { return m_Displacement != nullptr; }
		void SetDisplacement(const std::string displacement);
		void SetDisplacement(Ref<Texture> texture) { m_Displacement = texture; }

		json Serialize() override
		{
			BEGIN_SERIALIZE();
			
			j["Path"] = Path;
			j["UUID"] = static_cast<uint64_t>(ID);
			j["HasAlbedo"] = this->HasAlbedo();
			if (HasAlbedo())
			{
				j["Albedo"] = this->m_Albedo->Serialize();
			}
			Vector3 AlbedoColor = data.m_AlbedoColor;
			SERIALIZE_VEC3(AlbedoColor);

			SERIALIZE_VAL_LBL("Emissive", data.u_Emissive);
			SERIALIZE_VAL_LBL("AOValue", data.u_AOValue);
			SERIALIZE_VAL_LBL("MetalnessValue", data.u_MetalnessValue);
			SERIALIZE_VAL_LBL("RoughnessValue", data.u_RoughnessValue);
			SERIALIZE_VAL_LBL("Unlit", data.u_Unlit);

			j["HasAO"] = this->HasAO();
			if (HasAO())
			{
				j["AO"] = m_AO->Serialize();
			}

			j["HasMetalness"] = this->HasMetalness();
			if (HasMetalness())
			{
				j["Metalness"] = m_Metalness->Serialize();
			}

			j["HasRoughness"] = this->HasRoughness();
			if (HasRoughness())
			{
				j["Roughness"] = m_Roughness->Serialize();
			}

			j["HasNormal"] = this->HasNormal();
			if (HasNormal())
			{
				j["Normal"] = m_Normal->Serialize();
			}

			j["HasDisplacement"] = this->HasDisplacement();
			
			END_SERIALIZE();
		}

		bool Deserialize(const json& j) override
		{
			ID = static_cast<uint64_t>(j["UUID"]);

			if (j.contains("Albedo"))
			{
				const auto& texturePath = j["Albedo"]["Path"];
				const std::string absolutePath = FileSystem::RelativeToAbsolute(texturePath);

				if (FileSystem::FileExists(texturePath))
				{
					GPUResources& resources = GPUResources::Get();
					Ref<VulkanImage> image = CreateRef<VulkanImage>(absolutePath);
					if (resources.AddTexture(image))
					{
						AlbedoImage = image->GetID();
					}

					Ref<Texture> albedoTexture = TextureManager::Get()->GetTexture(absolutePath);
					SetAlbedo(albedoTexture);
				}
				else
				{
					GPUResources& resources = GPUResources::Get();
					Ref<VulkanImage> missingTexture = TextureManager::Get()->GetTexture2("missing_texture");
					AlbedoImage = missingTexture->GetID();
				}
			}

			if (j.contains("AlbedoColor"))
			{
				DESERIALIZE_VEC3(j["AlbedoColor"], data.m_AlbedoColor);
			}

			if (j.contains("Emissive"))
			{
				data.u_Emissive = j["Emissive"];
			}

			if (j.contains("MetalnessValue"))
			{
				data.u_MetalnessValue = j["MetalnessValue"];
			}

			if (j.contains("RoughnessValue"))
			{
				data.u_RoughnessValue = j["RoughnessValue"];
			}

			if (j.contains("AOValue"))
			{
				data.u_AOValue = j["AOValue"];
			}

			if (j.contains("Unlit"))
			{
				data.u_Unlit = j["Unlit"];
			}

			if (j.contains("Normal"))
			{
				const auto& texturePath = j["Normal"]["Path"];
				const std::string absolutePath = FileSystem::RelativeToAbsolute(texturePath);
				if (FileSystem::FileExists(texturePath))
				{
					GPUResources& resources = GPUResources::Get();
					Ref<VulkanImage> image = CreateRef<VulkanImage>(absolutePath);
					if (resources.AddTexture(image))
					{
						NormalImage = image->GetID();
					}
				}
				
				Ref<Texture> normalTexture = TextureManager::Get()->GetTexture(absolutePath);
				SetMetalness(normalTexture);
			}

			if (j.contains("AO"))
			{
				const auto& texturePath = j["AO"]["Path"];
				const std::string absolutePath = FileSystem::RelativeToAbsolute(texturePath);
				if (FileSystem::FileExists(texturePath))
				{
					GPUResources& resources = GPUResources::Get();
					Ref<VulkanImage> image = CreateRef<VulkanImage>(absolutePath);
					if (resources.AddTexture(image))
					{
						AOImage = image->GetID();
					}
				}

				Ref<Texture> aoTexture = TextureManager::Get()->GetTexture(absolutePath);
				SetAO(aoTexture);
			}

			if (j.contains("Metalness"))
			{
				const auto& texturePath = j["Metalness"]["Path"];
				const std::string absolutePath = FileSystem::RelativeToAbsolute(texturePath);
				if (FileSystem::FileExists(texturePath))
				{
					GPUResources& resources = GPUResources::Get();
					Ref<VulkanImage> image = CreateRef<VulkanImage>(absolutePath);
					if (resources.AddTexture(image))
					{
						MetalnessImage = image->GetID();
					}
				}

				Ref<Texture> metalTexture = TextureManager::Get()->GetTexture(absolutePath);
				SetMetalness(metalTexture);
			}

			if (j.contains("Roughness"))
			{
				const auto& texturePath = j["Roughness"]["Path"];
				const std::string absolutePath = FileSystem::RelativeToAbsolute(texturePath);
				if (FileSystem::FileExists(texturePath))
				{
					GPUResources& resources = GPUResources::Get();
					Ref<VulkanImage> image = CreateRef<VulkanImage>(absolutePath);
					if (resources.AddTexture(image))
					{
						RoughnessImage = image->GetID();
					}
				}

				Ref<Texture> metalTexture = TextureManager::Get()->GetTexture(absolutePath);
				SetRoughness(metalTexture);
			}

			if (j.contains("Displacement"))
			{
				const std::string absolutePath = FileSystem::RelativeToAbsolute(j["Normal"]["Path"]);
				Ref<Texture> displacementTexture = TextureManager::Get()->GetTexture(absolutePath);
				SetDisplacement(displacementTexture);
			}

			return true;
		}
	};
}
