#pragma once
#include "src/Rendering/Textures/TextureManager.h"
#include "Texture.h"

#include "src/Core/Core.h"
#include "src/Core/Maths.h"

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
	};

	class Material
	{
	private:
		std::string m_Name;
		unsigned int UBO;
	public:
		Ref<Texture> m_Albedo;
		Ref<Texture> m_AO;
		Ref<Texture> m_Metalness;
		Ref<Texture> m_Roughness;
		Ref<Texture> m_Normal;
		Ref<Texture> m_Displacement;

		UBOStructure data
		{
			0, // Has albedo texture
			0, // Padding byte
			0, // Padding byte
			0, // Padding byte
			Vector3(0.f, 0.f, 0.f),	// Albedo color
			0,						// Has metalness 
			0.5f,					// Metalness value
			0,						// u_HasRoughness
			0.5f,					// u_RoughnessValue
			0,						// u_HasAO
			0.5f,					// u_AOValue
			0,						// u_HasNormal
			0						// u_HasDisplacement
		};

		static Ref<Texture> m_DefaultAlbedo;
		static Ref<Texture> m_DefaultAO;
		static Ref<Texture> m_DefaultMetalness;
		static Ref<Texture> m_DefaultRoughness;
		static Ref<Texture> m_DefaultNormal;
		static Ref<Texture> m_DefaultDisplacement;

		Material(const std::string albedo);
		Material(Ref<Texture> texture) { m_Albedo = texture; }
		Material(const glm::vec3 albedoColor);

		~Material();

		void Bind();

		void SetupUniformBuffer();

		void SetName(const std::string name);
		std::string GetName();

		bool HasAlbedo() { return m_Albedo != nullptr; }
		void SetAlbedo(const std::string path) { m_Albedo = CreateRef<Texture>(path); }
		void SetAlbedo(Ref<Texture> texture) { m_Albedo = texture; }

		bool HasAO() { return m_AO != nullptr; }
		void SetAO(const std::string albedo);
		void SetAO(Ref<Texture> texture) { m_AO = texture; }

		bool HasMetalness() { return m_Metalness != nullptr; }
		void SetMetalness(const std::string albedo);
		void SetMetalness(Ref<Texture> texture) { m_Metalness = texture; }

		bool HasRougness() { return m_Roughness != nullptr; }
		void SetRoughness(const std::string albedo);
		void SetRoughness(Ref<Texture> texture) { m_Roughness = texture; }

		bool HasNormal() { return m_Normal != nullptr; }
		void SetNormal(const std::string albedo);
		void SetNormal(Ref<Texture> texture) { m_Normal = texture; }

		bool HasDisplacement() { return m_Displacement != nullptr; }
		void SetDisplacement(const std::string displacement);
		void SetDisplacement(Ref<Texture> texture) { m_Displacement = texture; }
	};
}
