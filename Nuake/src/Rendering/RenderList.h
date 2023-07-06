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
		uint32_t entityId;
	};

	class RenderList
	{
	public:
		RenderList()
		{
			this->m_RenderList = std::map<Ref<Material>, std::vector<RenderMesh>>();
		}

		void AddToRenderList(Ref<Mesh> mesh, Matrix4 transform, const uint32_t entityId = -1)
		{
			Ref<Material> material = mesh->GetMaterial();

			if (m_RenderList.find(material) == m_RenderList.end())
				m_RenderList[material] = std::vector<RenderMesh>();

			m_RenderList[material].push_back({mesh, transform, entityId});
		}

		void Flush(Shader* shader, bool depthOnly = false)
		{
			shader->Bind();
			for (auto& i : m_RenderList)
			{
				if(!depthOnly)
					i.first->Bind(shader);

				for (auto& m : i.second)
				{
					shader->SetUniformMat4f("u_Model", m.transform);
					shader->SetUniform1i("u_EntityID", m.entityId);
					m.Mesh->Draw(shader, false);
				}
			}

			m_RenderList.clear();
		}

	private:
		std::map<Ref<Material>, std::vector<RenderMesh>> m_RenderList;
	};
}
