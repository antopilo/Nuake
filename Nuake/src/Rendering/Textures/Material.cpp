

#include "../Core/MaterialManager.h"
#include "Material.h"
#include <glm\ext\vector_float4.hpp>
#include "../Renderer.h"
#include <memory>
#include <string>
#include <sstream>
#include <vector>
Ref<Texture> Material::m_DefaultAlbedo;
Ref<Texture> Material::m_DefaultAO;
Ref<Texture> Material::m_DefaultNormal;
Ref<Texture> Material::m_DefaultRoughness;
Ref<Texture> Material::m_DefaultMetalness;
Ref<Texture> Material::m_DefaultDisplacement;


Material::Material(const std::string albedo) 
{

	m_Albedo = TextureManager::Get()->GetTexture(albedo);

	UseAlbedo = true;

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
	m_AlbedoColor = albedoColor;
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

Material::~Material() 
{

}

void Material::Bind() 
{
	
	// Albedo
	if (m_Albedo != nullptr)
		m_Albedo->Bind(4);
	else
		m_DefaultAlbedo->Bind(4);
	Renderer::m_Shader->SetUniform1i("m_Albedo", 4);
	Renderer::m_Shader->SetUniform1i("u_HasAlbedo", UseAlbedo);
	Renderer::m_Shader->SetUniform3f("m_AlbedoColor", m_AlbedoColor.r, m_AlbedoColor.g, m_AlbedoColor.b);
	Renderer::m_GBufferShader->SetUniform1i("m_Albedo", 4);

	// AO
	if (m_AO != nullptr)
		m_AO->Bind(5);
	else
		m_DefaultAO->Bind(5);
	Renderer::m_Shader->SetUniform1i("m_AO", 5);
	Renderer::m_Shader->SetUniform1i("u_HasAO", UseAO);
	Renderer::m_Shader->SetUniform1f("u_AOValue", m_AOValue);
	Renderer::m_GBufferShader->SetUniform1i("m_AO", 5);
	// Metallic
	if (m_Metalness != nullptr) 
		m_Metalness->Bind(6);
	else 
		m_DefaultMetalness->Bind(6);
	Renderer::m_Shader->SetUniform1i("m_Metalness", 6);
	Renderer::m_Shader->SetUniform1i("u_HasMetalness", UseMetalness);
	Renderer::m_Shader->SetUniform1f("u_MetalnessValue", m_MetalnessValue);
	Renderer::m_GBufferShader->SetUniform1i("m_Metalness", 6);
	// Roughness
	if (m_Roughness != nullptr)
		m_Roughness->Bind(7);
	else
		m_DefaultRoughness->Bind(7);
	Renderer::m_Shader->SetUniform1i("m_Roughness", 7);
	Renderer::m_Shader->SetUniform1i("u_HasRoughness", UseRoughness);
	Renderer::m_Shader->SetUniform1f("u_RoughnessValue", m_RoughnessValue);
	Renderer::m_GBufferShader->SetUniform1i("m_Roughness", 7);
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

