#include "BoxGizmo.h"
#include "Nuake/Core/Maths.h"


namespace Nuake {

	void BoxGizmo::Bind()
	{
		_VertexArray->Bind();
	}

	void BoxGizmo::UpdateShape(float radius, float height)
	{
		CreateMesh();
	}

	void BoxGizmo::CreateMesh()
	{
		const Color cubeColor = Color(1, 0, 0, 0.5f);
		std::vector<LineVertex> mBoxVertices =
		{
			LineVertex{Vector3(-1.f, -1.f, -1.f), cubeColor},
			LineVertex{Vector3(1.0f, -1.f, -1.f), cubeColor},
			LineVertex{Vector3(-1.f, -1.f, 1.f), cubeColor},
			LineVertex{Vector3(1.0f, -1.f, 1.f), cubeColor},
			LineVertex{Vector3(-1.f, 1.f, -1.f), cubeColor},
			LineVertex{Vector3(1.0f, 1.f, -1.f), cubeColor},
			LineVertex{Vector3(-1.f, 1.f, 1.f), cubeColor},
			LineVertex{Vector3(1.0f, 1.f, 1.f), cubeColor},

			LineVertex{Vector3(-1.f, -1.f, -1.f), cubeColor},
			LineVertex{Vector3(-1.0f, -1.f, 1.f), cubeColor},
			LineVertex{Vector3(1.f, -1.f, -1.f), cubeColor},
			LineVertex{Vector3(1.0f, -1.f, 1.f), cubeColor},
			LineVertex{Vector3(-1.f, 1.f, -1.f), cubeColor},
			LineVertex{Vector3(-1.0f, 1.f, 1.f), cubeColor},
			LineVertex{Vector3(1.f, 1.f, -1.f), cubeColor},
			LineVertex{Vector3(1.0f, 1.f, 1.f), cubeColor},

			LineVertex{Vector3(-1.0f, -1.0f, -1.f), cubeColor},
			LineVertex{Vector3(-1.f, 1.0f, -1.f), cubeColor},
			LineVertex{Vector3(1.0f, -1.0f, -1.f), cubeColor},
			LineVertex{Vector3(1.f, 1.0f, -1.f), cubeColor},
			LineVertex{Vector3(-1.0f, -1.0f, 1.f), cubeColor},
			LineVertex{Vector3(-1.f, 1.0f, 1.f), cubeColor},
			LineVertex{Vector3(1.0f, -1.0f, 1.f), cubeColor},
			LineVertex{Vector3(1.0f, 1.0f, 1.f), cubeColor}
		};

		_Vertices = mBoxVertices;

		_VertexArray = CreateRef<VertexArray>();
		_VertexArray->Bind();

		_VertexBuffer = CreateRef<VertexBuffer>(_Vertices.data(), _Vertices.size() * sizeof(LineVertex));

		auto vblayout = CreateRef<VertexBufferLayout>();
		vblayout->Push<float>(3);
		vblayout->Push<float>(4);

		_VertexArray->AddBuffer(*_VertexBuffer, *vblayout);
	}

}