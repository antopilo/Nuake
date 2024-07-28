#include "CylinderGizmo.h"

#include <src/Core/Maths.h>

CylinderGizmo::CylinderGizmo(Nuake::Color color) :
	_Radius(0.0f),
	_Height(0.0f),
	_Color(color)
{
}

void CylinderGizmo::Bind()
{
	_CylinderBuffer->Bind();
}

void CylinderGizmo::UpdateShape(float radius, float height, Nuake::Color color)
{
	if (_Radius == radius && _Height == height && _Color == color)
	{
		return;
	}

	_Radius = radius;
	_Height = height;
	_Color = color;

	CreateMesh();
}

void CylinderGizmo::CreateMesh()
{
	using namespace Nuake;

	_CylinderVertices.clear();

	const float halfHeight = _Height / 2.0f;
	const float radius = _Radius;
	const float bottomCircleHeight = -halfHeight;
	const float topCircleHeight = halfHeight;

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

		_CylinderVertices.push_back(LineVertex{ vert1, _Color });
		_CylinderVertices.push_back(LineVertex{ vert2, _Color });
	}

	_CylinderVertices.push_back(LineVertex{ Vector3(radius, bottomCircleHeight, 0), _Color });
	_CylinderVertices.push_back(LineVertex{ Vector3(radius, topCircleHeight, 0), _Color });

	_CylinderVertices.push_back(LineVertex{ Vector3(-radius, bottomCircleHeight, 0), _Color });
	_CylinderVertices.push_back(LineVertex{ Vector3(-radius, topCircleHeight, 0), _Color });

	_CylinderVertices.push_back(LineVertex{ Vector3(0, bottomCircleHeight, radius), _Color });
	_CylinderVertices.push_back(LineVertex{ Vector3(0, topCircleHeight, radius), _Color });

	_CylinderVertices.push_back(LineVertex{ Vector3(0, bottomCircleHeight, -radius), _Color });
	_CylinderVertices.push_back(LineVertex{ Vector3(0, topCircleHeight, -radius), _Color });

	_CylinderBuffer = CreateRef<Nuake::VertexArray>();
	_CylinderBuffer->Bind();

	_CylinderVertexBuffer = CreateRef<VertexBuffer>(_CylinderVertices.data(), _CylinderVertices.size() * sizeof(Nuake::LineVertex));
	auto vblayout = CreateRef<VertexBufferLayout>();
	vblayout->Push<float>(3);
	vblayout->Push<float>(4);

	_CylinderBuffer->AddBuffer(*_CylinderVertexBuffer, *vblayout);
}