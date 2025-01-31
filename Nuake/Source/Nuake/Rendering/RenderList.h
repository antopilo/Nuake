#pragma once
#include <vector>
#include <map>
#include "Nuake/Core/Maths.h"
#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Textures/Material.h"
#include "Nuake/Rendering/Mesh/Mesh.h"
#include "Nuake/Rendering/Shaders/ShaderManager.h"
#include "Nuake/Rendering/Textures/MaterialManager.h"

namespace Nuake
{
	struct RenderMesh
	{
		Ref<Mesh> mesh;
		Matrix4 transform;
		int32_t entityId;
		Matrix4 previousTransform;
	};

	class RenderList
	{
	public:
		RenderList()
		{
			this->m_RenderList = std::unordered_map<Ref<Material>, std::vector<RenderMesh>>();
		}

		void AddToRenderList(Ref<Mesh> mesh, const Matrix4& transform, const int32_t entityId = -1, const Matrix4& previousTransform = Matrix4(0.0f) )
		{
			Ref<Material> material = mesh->GetMaterial();

			if (!material)
			{
				material = MaterialManager::Get()->GetMaterial("default");
			}

			if (m_RenderList.find(material) == m_RenderList.end())
			{
				m_RenderList[material] = std::vector<RenderMesh>();
			}

			m_RenderList[material].push_back({std::move(mesh), std::move(transform), entityId, previousTransform});
		}

		void Flush(Shader* shader, bool depthOnly = false)
		{
			shader->Bind();
			uint32_t entityIdUniformLocation = -1;;
			if (!depthOnly)
			{
				entityIdUniformLocation = shader->FindUniformLocation("u_EntityID");
			}

			const uint32_t modelMatrixUniformLocation = shader->FindUniformLocation("u_Model");
			const int previousTransformUniformLocation = shader->FindUniformLocation("u_PreviousViewModel");
			for (auto& i : m_RenderList)
			{
				if (!depthOnly)
				{
					i.first->Bind(shader);
				}

				for (auto& m : i.second)
				{
					if (!depthOnly)
					{
						shader->SetUniform(entityIdUniformLocation, m.entityId + 1);
					}

					if (previousTransformUniformLocation != -1)
					{
						shader->SetUniform(previousTransformUniformLocation, m.previousTransform);
					}

					shader->SetUniform(modelMatrixUniformLocation, m.transform);
					m.mesh->Draw(shader, false);
				}
			}

			m_RenderList.clear();
		}

	private:
		std::unordered_map<Ref<Material>, std::vector<RenderMesh>> m_RenderList;
	};
}
