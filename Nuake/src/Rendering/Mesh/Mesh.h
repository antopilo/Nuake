#pragma once
#include <vector>
#include "../Textures/Texture.h"
#include "../Vertex.h"
#include <glm\ext\matrix_float4x4.hpp>
#include "../Textures/Material.h"
#include "../Core/Core.h"
struct MeshVertex
{

};


class __declspec(dllexport) Mesh
{
public:
	std::vector<Texture> m_Textures;
	std::vector<unsigned int> m_Indices;
	std::vector<Vertex> m_Vertices;
	Ref<Material> m_Material;


	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Ref<Material> material);
	void Draw();
	void DebugDraw();
private:
	//  render data
	unsigned int VAO, VBO, EBO;
	void setupMesh();
};