#pragma once
#include "../../../Rendering/Textures/Material.h"
#include <glm\ext\matrix_float4x4.hpp>
#include "BaseComponent.h"

class MeshComponent  {

private:
	unsigned int VAO;
	unsigned int VBO;
	Ref<Material> m_Material;

	void BuildTangents();
public:
	void LoadModel(const std::string path);
	//void ProcessNode(aiNode* node, const aiScene* scene);
	MeshComponent();

	void SetMaterial(const std::string materialName);
	void Draw(glm::mat4 projection, glm::mat4 view, glm::mat4 transform);
	void DrawEditor();

	void RenderSphere();
};