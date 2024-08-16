#include "CapsuleGizmo.h"

#include <src/Core/Maths.h>

namespace Nuake {

	CapsuleGizmo::CapsuleGizmo() :
		_Radius(0.0f),
		_Height(0.0f)
	{
	}

	void CapsuleGizmo::Bind()
	{
		_CapsuleBuffer->Bind();
	}

	void CapsuleGizmo::UpdateShape(float radius, float height)
	{
		if (_Radius == radius && _Height == height)
		{
			return;
		}

		_Radius = radius;
		_Height = height;
		CreateMesh();
	}

	void CapsuleGizmo::CreateMesh()
	{
		_CapsuleVertices.clear();

		const float halfHeight = _Height / 2.0f;
		const float radius = _Radius;
		const float bottomCircleHeight = -halfHeight + radius;
		const float topCircleHeight = halfHeight - radius;

		// Generate circles
		const float subDivision = 32.0f;
		constexpr const float pi = glm::pi<float>() * 4.0;
		float increment = pi / subDivision;
		for (int i = 0; i < subDivision * 2.0; i++)
		{
			float current = increment * (i);
			float x = glm::cos(current) * radius;
			float z = glm::sin(current) * radius;

			current = increment * (i + 1);
			float x2 = glm::cos(current) * radius;
			float z2 = glm::sin(current) * radius;

			Vector3 vert1, vert2;
			if (i < subDivision)
			{
				vert1 = Vector3(x, topCircleHeight, z);
				vert2 = Vector3(x2, topCircleHeight, z2);
			}
			else
			{
				vert1 = Vector3(x, bottomCircleHeight, z);
				vert2 = Vector3(x2, bottomCircleHeight, z2);
			}

			_CapsuleVertices.push_back(LineVertex{ vert1, Color(1.0, 0, 0.0, 0.5) });
			_CapsuleVertices.push_back(LineVertex{ vert2, Color(1.0, 0, 0.0, 0.5) });
		}

		for (int i = 0; i < subDivision * 2.0; i++)
		{
			float current = increment * (i);
			float x = glm::cos(current) * radius;
			float z = glm::sin(current) * radius;

			current = increment * (i + 1);
			float x2 = glm::cos(current) * radius;
			float z2 = glm::sin(current) * radius;

			float heightOffset = topCircleHeight;
			if (z < 0.0)
			{
				heightOffset = bottomCircleHeight;
			}

			Vector3 vert1, vert2;
			if (i < subDivision)
			{
				vert1 = Vector3(x, z + heightOffset, 0);
				vert2 = Vector3(x2, z2 + heightOffset, 0);
			}
			else
			{
				vert1 = Vector3(0, z + heightOffset, x);
				vert2 = Vector3(0, z2 + heightOffset, x2);
			}

			_CapsuleVertices.push_back(LineVertex{ vert1, Color(1.0, 0, 0.0, 0.5) });
			_CapsuleVertices.push_back(LineVertex{ vert2, Color(1.0, 0, 0.0, 0.5) });
		}

		_CapsuleVertices.push_back(LineVertex{ Vector3(radius, bottomCircleHeight, 0), Color(1.0, 0, 0.0, 0.5) });
		_CapsuleVertices.push_back(LineVertex{ Vector3(radius, topCircleHeight, 0), Color(1.0, 0, 0.0, 0.5) });

		_CapsuleVertices.push_back(LineVertex{ Vector3(-radius, bottomCircleHeight, 0), Color(1.0, 0, 0.0, 0.5) });
		_CapsuleVertices.push_back(LineVertex{ Vector3(-radius, topCircleHeight, 0), Color(1.0, 0, 0.0, 0.5) });

		_CapsuleVertices.push_back(LineVertex{ Vector3(0, bottomCircleHeight, radius), Color(1.0, 0, 0.0, 0.5) });
		_CapsuleVertices.push_back(LineVertex{ Vector3(0, topCircleHeight, radius), Color(1.0, 0, 0.0, 0.5) });

		_CapsuleVertices.push_back(LineVertex{ Vector3(0, bottomCircleHeight, -radius), Color(1.0, 0, 0.0, 0.5) });
		_CapsuleVertices.push_back(LineVertex{ Vector3(0, topCircleHeight, -radius), Color(1.0, 0, 0.0, 0.5) });

		_CapsuleBuffer = CreateRef<Nuake::VertexArray>();
		_CapsuleBuffer->Bind();

		_CapsuleVertexBuffer = CreateRef<VertexBuffer>(_CapsuleVertices.data(), _CapsuleVertices.size() * sizeof(Nuake::LineVertex));
		auto vblayout = CreateRef<VertexBufferLayout>();
		vblayout->Push<float>(3);
		vblayout->Push<float>(4);

		_CapsuleBuffer->AddBuffer(*_CapsuleVertexBuffer, *vblayout);
	}
}
