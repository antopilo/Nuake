

#include "../Core/MaterialManager.h"
#include "Material.h"
#include <glm\ext\vector_float4.hpp>
#include "../Renderer.h"
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <dependencies/GLEW/include/GL/glew.h>
Ref<Texture> Material::m_DefaultAlbedo;
Ref<Texture> Material::m_DefaultAO;
Ref<Texture> Material::m_DefaultNormal;
Ref<Texture> Material::m_DefaultRoughness;
Ref<Texture> Material::m_DefaultMetalness;
Ref<Texture> Material::m_DefaultDisplacement;


Material::Material(const std::string albedo) 
{
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOStructure), NULL, GL_STATIC_DRAW); 
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	m_Albedo = TextureManager::Get()->GetTexture(albedo);

	data.u_HasAlbedo = 1;

	std::stringstream ss(albedo);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, '/')) {
		elems.push_back(item);
		// elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
	}

	m_Name = albedo;
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

//Material::Material()
//{
//}

Material::Material(const glm::vec3 albedoColor)
{
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 128, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	data.m_AlbedoColor = Vector3{ albedoColor.r, albedoColor.g, albedoColor.b};

	m_Name = "New material";

	if (m_DefaultAlbedo == nullptr)
		m_DefaultAlbedo = TextureManager::Get()->GetTexture("resources/Textures/default/Default.png");

	m_Albedo = m_DefaultAlbedo;

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

Material::~Material() {}

void Material::Bind() 
{
	if (MaterialManager::Get()->CurrentlyBoundedMaterial == m_Name)
		return;

	MaterialManager::Get()->CurrentlyBoundedMaterial = m_Name;
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBOStructure), &data);
	//glBufferSubData(GL_UNIFORM_BUFFER, 0, 4, &data.u_HasAlbedo);
	//
	//glBufferSubData(GL_UNIFORM_BUFFER, 16, 12, &data.m_AlbedoColor);
	//
	//glBufferSubData(GL_UNIFORM_BUFFER, 28, 4, &data.u_HasMetalness);
	//glBufferSubData(GL_UNIFORM_BUFFER, 32, 4, &data.u_MetalnessValue);
	//glBufferSubData(GL_UNIFORM_BUFFER, 36, 4, &data.u_HasRoughness);
	//glBufferSubData(GL_UNIFORM_BUFFER, 40, 4, &data.u_RoughnessValue);
	//
	//glBufferSubData(GL_UNIFORM_BUFFER, 44, 4, &data.u_HasAO);
	//glBufferSubData(GL_UNIFORM_BUFFER, 48, 4, &data.u_AOValue);
	//glBufferSubData(GL_UNIFORM_BUFFER, 52, 4, &data.u_HasNormal);
	//glBufferSubData(GL_UNIFORM_BUFFER, 56, 4, &data.u_HasDisplacement);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 32, UBO);

	// Albedo
	if (m_Albedo != nullptr)
		m_Albedo->Bind(4);
	else
		m_DefaultAlbedo->Bind(4);
	Renderer::m_Shader->SetUniform1i("m_Albedo", 4);

	// AO
	if (m_AO != nullptr)
		m_AO->Bind(5);
	else
		m_DefaultAO->Bind(5);
	Renderer::m_Shader->SetUniform1i("m_AO", 5);

	// Metallic
	if (m_Metalness != nullptr) 
		m_Metalness->Bind(6);
	else 
		m_DefaultMetalness->Bind(6);
	Renderer::m_Shader->SetUniform1i("m_Metalness", 6);

	// Roughness
	if (m_Roughness != nullptr)
		m_Roughness->Bind(7);
	else
		m_DefaultRoughness->Bind(7);
	Renderer::m_Shader->SetUniform1i("m_Roughness", 7);

	// Normal
	if (m_Normal != nullptr)
		m_Normal->Bind(8);
	else
		m_DefaultNormal->Bind(8);
	Renderer::m_Shader->SetUniform1i("m_Normal", 8);

	// Displacement
	if (m_Displacement != nullptr)
		m_Displacement->Bind(9);
	else
		m_DefaultDisplacement->Bind(9);
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

