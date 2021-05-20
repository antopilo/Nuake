#pragma once
#include "../../Core/TextureManager.h"
#include "Texture.h"
#include <glm\ext\vector_float3.hpp>
#include "../Core/Core.h"
class Material
{
private:
	std::string m_Name;

public:
	Ref<Texture> m_Albedo;
	Ref<Texture> m_AO;
	Ref<Texture> m_Metalness;
	Ref<Texture> m_Roughness;
	Ref<Texture> m_Normal;
	Ref<Texture> m_Displacement;

	bool UseAlbedo = false;
	bool UseNormal = false;
	bool UseMetalness = false;
	bool UseRoughness = false;
	bool UseDisplacement = false;
	bool UseAO = false;

	static Ref<Texture> m_DefaultAlbedo;
	static Ref<Texture> m_DefaultAO;
	static Ref<Texture> m_DefaultMetalness;
	static Ref<Texture> m_DefaultRoughness;
	static Ref<Texture> m_DefaultNormal;
	static Ref<Texture> m_DefaultDisplacement;

	glm::vec3 m_AlbedoColor = glm::vec3(1.f, 1.f, 1.f);
	float m_AOValue = 1.0f;
	float m_MetalnessValue = 0.5f;
	float m_RoughnessValue = 0.5f;


	Material(const std::string albedo);
	Material(Ref<Texture> texture) { m_Albedo = texture; }
	Material(const glm::vec3 albedoColor);
	~Material();

	void Bind();

	void SetName(const std::string name);
	std::string GetName();

	void SetAlbedo(const std::string path) { m_Albedo = CreateRef<Texture>(path); }
	void SetAlbedo(Ref<Texture> texture) { m_Albedo = texture; }

	void SetAO(const std::string albedo);
	void SetAO(Ref<Texture> texture) { m_AO = texture; }

	void SetMetalness(const std::string albedo);
	void SetMetalness(Ref<Texture> texture) { m_Metalness = texture; }

	void SetRoughness(const std::string albedo);
	void SetRoughness(Ref<Texture> texture) { m_Roughness = texture; }

	void SetNormal(const std::string albedo);
	void SetNormal(Ref<Texture> texture) { m_Normal = texture; }

	void SetDisplacement(const std::string displacement);
	void SetDisplacement(Ref<Texture> texture) { m_Displacement = texture; }

	bool HasAlbedo() { return m_Albedo != nullptr; }
	bool HasNormal() { return m_Normal != nullptr; }
	bool HasAO() { return m_AO != nullptr; }
	bool HasMetalness() { return m_Metalness != nullptr; }
	bool HasRougness() { return m_Roughness != nullptr; }
	bool HasDisplacement() { return m_Displacement != nullptr; }
};