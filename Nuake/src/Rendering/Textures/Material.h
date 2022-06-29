#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include "src/Rendering/Shaders/Shader.h"
#include "src/Rendering/Textures/TextureManager.h"
#include "Texture.h"
#include <src/Resource/Serializable.h>
#include "src/Resource/Resource.h"
#include "src/Core/FileSystem.h"

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
				0.f,					// u_RoughnessValue
				0,						// u_HasAO
				1.f,					// u_AOValue
				0,						// u_HasNormal
				0,						// u_HasDisplacement
				0
			};
		}
	public:
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
		Material(const glm::vec3 albedoColor);
		~Material();

		void InitDefaultTextures();

		void Bind(Shader* shader);
		void SetupUniformBuffer();

		void SetName(const std::string name);
		std::string GetName();

		inline void SetUnlit(bool value) { data.u_Unlit = value; }
		inline bool GetUnlit() { return data.u_Unlit == 1; }

		bool HasAlbedo() { return m_Albedo != nullptr; }
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
			j["HasAlbedo"] = this->HasAlbedo();
			if (HasAlbedo())
			{
				j["Albedo"] = this->m_Albedo->Serialize();
			}
			j["HasAO"] = this->HasAO();
			j["HasMetalness"] = this->HasMetalness();
			j["HasRoughness"] = this->HasRoughness();
			j["HasNormal"] = this->HasNormal();
			j["HasDisplacement"] = this->HasDisplacement();
			
			END_SERIALIZE();
		}

		bool Deserialize(const std::string& str) override
		{
			BEGIN_DESERIALIZE();

			if (j.contains("Path"))
			{
				this->Path = j["Path"];
				if (FileSystem::FileExists(Path))
				{
					std::string content = FileSystem::ReadFile(Path);
					Deserialize(content);
				}
			}
			else
			{
				if (j.contains("Albedo"))
				{
					Ref<Texture> albedoTexture = TextureManager::Get()->GetTexture(j["Albedo"]["Path"]);
					SetAlbedo(albedoTexture);
				}

				if (j.contains("Normal"))
				{
					Ref<Texture> normalTexture = TextureManager::Get()->GetTexture(j["Normal"]["Path"]);
					SetMetalness(normalTexture);
				}

				if (j.contains("AO"))
				{
					Ref<Texture> aoTexture = TextureManager::Get()->GetTexture(j["AO"]["Path"]);
					SetAO(aoTexture);
				}

				if (j.contains("Metalness"))
				{
					Ref<Texture> metalTexture = TextureManager::Get()->GetTexture(j["Metalness"]["Path"]);
					SetMetalness(metalTexture);
				}

				if (j.contains("Roughness"))
				{
					Ref<Texture> metalTexture = TextureManager::Get()->GetTexture(j["Roughness"]["Path"]);
					SetRoughness(metalTexture);
				}

				if (j.contains("Displacement"))
				{
					Ref<Texture> displacementTexture = TextureManager::Get()->GetTexture(j["Displacement"]["Path"]);
					SetDisplacement(displacementTexture);
				}
			}

			return true;
		}
	};
}
