#include "src/Rendering/Textures/MaterialManager.h"
#include "Material.h"

#include "src/Core/Maths.h"
#include "src/Rendering/Renderer.h"

#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include <GL/glew.h>

namespace Nuake
{
	Ref<Texture> Material::m_DefaultAlbedo;
	Ref<Texture> Material::m_DefaultAO;
	Ref<Texture> Material::m_DefaultNormal;
	Ref<Texture> Material::m_DefaultRoughness;
	Ref<Texture> Material::m_DefaultMetalness;
	Ref<Texture> Material::m_DefaultDisplacement;

	Material::Material() 
	{
		InitDefaultTextures();
		InitUniformBuffer();

		glGenBuffers(1, &UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOStructure), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	Material::Material(const std::string albedo)
	{
		InitUniformBuffer();

		glGenBuffers(1, &UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOStructure), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		m_Albedo = TextureManager::Get()->GetTexture(albedo);

		data.u_HasAlbedo = 1;

		std::stringstream ss(albedo);
		std::string item;
		std::vector<std::string> elems;
		while (std::getline(ss, item, '/')) 
		{
			elems.push_back(item);
		}

		m_Name = albedo;
		InitDefaultTextures();
	}

	Material::Material(const Vector3 albedoColor)
	{
		InitDefaultTextures();
		InitUniformBuffer();

		glGenBuffers(1, &UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferData(GL_UNIFORM_BUFFER, 128, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		data.m_AlbedoColor = Vector3{ albedoColor.r, albedoColor.g, albedoColor.b };

		m_Name = "New material";

		m_Albedo = m_DefaultAlbedo;
	}

	Material::~Material() {}

	void Material::InitDefaultTextures()
	{
		if (m_DefaultAlbedo == nullptr)
			m_DefaultAlbedo = TextureManager::Get()->GetTexture("resources/Textures/default/Default.png");
		if (m_DefaultAO == nullptr)
			m_DefaultAO = TextureManager::Get()->GetTexture("resources/Textures/default/Default.png");
		if (m_DefaultNormal == nullptr)
			m_DefaultNormal = TextureManager::Get()->GetTexture("resources/Textures/default/defaultNormal.png");
		if (m_DefaultDisplacement == nullptr)
			m_DefaultDisplacement = TextureManager::Get()->GetTexture("resources/Textures/default/Default.png");
		if (m_DefaultRoughness == nullptr)
			m_DefaultRoughness = TextureManager::Get()->GetTexture("resources/Textures/default/Default.png");
		if (m_DefaultMetalness == nullptr)
			m_DefaultMetalness = TextureManager::Get()->GetTexture("resources/Textures/default/Default.png");
	}

	void Material::Bind(Shader* shader)
	{
		//if (MaterialManager::Get()->CurrentlyBoundedMaterial == m_Name)
		//	return;

		MaterialManager::Get()->CurrentlyBoundedMaterial = m_Name;
		

		// Albedo
		if (m_Albedo != nullptr)
		{
			data.u_HasAlbedo = 1;
			m_Albedo->Bind(4);
		}
		else
			m_DefaultAlbedo->Bind(4);
		shader->SetUniform1i("m_Albedo", 4);

		// AO
		if (m_AO != nullptr)
		{
			data.u_HasAO = 1;
			m_AO->Bind(5);
		}
		else
			m_DefaultAO->Bind(5);
		shader->SetUniform1i("m_AO", 5);

		// Metallic
		if (m_Metalness != nullptr) {
			data.u_HasMetalness = 1; 
			m_Metalness->Bind(6);
		}
		else
			m_DefaultMetalness->Bind(6);
		shader->SetUniform1i("m_Metalness", 6);

		// Roughness
		if (m_Roughness != nullptr) {
			data.u_HasRoughness = 1;
			m_Roughness->Bind(7);
		}
		else
			m_DefaultRoughness->Bind(7);
		shader->SetUniform1i("m_Roughness", 7);

		// Normal
		if (m_Normal != nullptr) {
			data.u_HasNormal = 1;
			m_Normal->Bind(8);
		}
		else
			m_DefaultNormal->Bind(8);

		shader->SetUniform1i("m_Normal", 8);

		// Displacement
		if (m_Displacement != nullptr)
			m_Displacement->Bind(9);
		else
			m_DefaultDisplacement->Bind(9);

		glBindBuffer(GL_UNIFORM_BUFFER, UBO);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBOStructure), &data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 32, UBO);
		//Renderer::m_Shader->SetUniform1i("m_Displacement", 9);
		//Renderer::m_Shader->SetUniform1i("m_Displacement", 9);
	}

	void Material::SetupUniformBuffer()
	{

	}

	void Material::SetName(const std::string name)
	{
		m_Name = name;
	}

	std::string Material::GetName() { return m_Name; }


	void Material::SetAO(const std::string ao)
	{
		m_AO = CreateRef<Texture>(ao);
	}

	void Material::SetMetalness(const std::string metalness)
	{
		m_Metalness = CreateRef<Texture>(metalness);
	}

	void Material::SetRoughness(const std::string roughness)
	{
		m_Roughness = CreateRef<Texture>(roughness);
	}

	void Material::SetNormal(const std::string normal)
	{
		m_Normal = CreateRef<Texture>(normal);
	}

	void Material::SetDisplacement(const std::string displacement)
	{
		m_Displacement = CreateRef<Texture>(displacement);
	}
}


