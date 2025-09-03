#include "SphereGizmo.h"

namespace Nuake {

	void SphereGizmo::Bind()
	{
		_VertexArray->Bind();
	}

	void SphereGizmo::CreateMesh()
	{
		const float subDivision = 32.0f;
		constexpr const float pi = glm::pi<float>() * 4.0;
		float increment = pi / subDivision;
		for (int i = 0; i < subDivision * 2.0; i++)
		{
			float current = increment * (i);
			float x = glm::cos(current) * 1.0;
			float z = glm::sin(current) * 1.0;

			current = increment * (i + 1);
			float x2 = glm::cos(current) * 1.0;
			float z2 = glm::sin(current) * 1.0;

			Vector3 vert1, vert2;
			if (i < subDivision)
			{
				vert1 = Vector3(x, z, 0);
				vert2 = Vector3(x2, z2, 0);
			}
			else
			{
				vert1 = Vector3(x, 0, z);
				vert2 = Vector3(x2, 0, z2);
			}

			_Vertices.push_back(LineVertex{ vert1, Color(1.0, 0, 0.0, 0.5) });
			_Vertices.push_back(LineVertex{ vert2, Color(1.0, 0, 0.0, 0.5) });
		}

		_VertexArray = CreateRef<Nuake::VertexArray>();
		_VertexArray->Bind();

		_VertexBuffer = CreateRef<VertexBuffer>(_Vertices.data(), _Vertices.size() * sizeof(Nuake::LineVertex));
		auto vblayout = CreateRef<VertexBufferLayout>();
		vblayout->Push<float>(3);
		vblayout->Push<float>(4);

		_VertexArray->AddBuffer(*_VertexBuffer, *vblayout);
	}
}

