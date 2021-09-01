#pragma once
#include <vector>
#include <map>
#include "src/Core/Maths.h"
#include "src/Core/Core.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Mesh/Mesh.h"
#include "src/Rendering/Shaders/ShaderManager.h"
namespace Nuake
{
	struct RenderMesh
	{
		Ref<Mesh> Mesh;
		Matrix4 transform;
	};

	class RenderList
	{
	public:
		RenderList()
		{
			this->m_RenderList = std::map<Ref<Material>, std::vector<RenderMesh>>();
		}

		void AddToRenderList(Ref<Mesh> mesh, Matrix4 transform)
		{
			if (m_RenderList.find(mesh->m_Material) == m_RenderList.end())
				m_RenderList[mesh->m_Material] = std::vector<RenderMesh>();

			m_RenderList[mesh->m_Material].push_back({mesh, transform});
		}

		void Flush(Ref<Shader> shader, bool depthOnly = false)
		{
			shader->Bind();
			for (auto& i : m_RenderList)
			{
				if(!depthOnly)
					i.first->Bind(shader);

				for (auto& m : i.second)
				{
					shader->SetUniformMat4f("u_Model", m.transform);
					m.Mesh->Draw(shader, false);
				}
			}

			m_RenderList.clear();
		}

	private:
		std::map<Ref<Material>, std::vector<RenderMesh>> m_RenderList;
	};
}
