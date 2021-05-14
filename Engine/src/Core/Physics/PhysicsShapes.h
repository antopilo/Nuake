#pragma once

#include <glm/ext/vector_float3.hpp>
#include "../../Rendering/Mesh/Mesh.h"
class btCollisionShape;
namespace Physics {
	enum RigidbodyShapes {
		BOX, SPHERE, CAPSULE, MESH
	};
	class __declspec(dllexport) PhysicShape {
	protected: 
		btCollisionShape* bShape;
		RigidbodyShapes m_Type;
	public:
		virtual btCollisionShape* GetBulletShape() = 0;
		RigidbodyShapes GetType() const { return m_Type; }
		
	};

	class __declspec(dllexport) Box : public PhysicShape {
	private:
		glm::vec3 Size;
		btCollisionShape* bShape;
	public:
		Box();
		Box(glm::vec3 size);
		Box(float x, float y, float z);

		glm::vec3 GetSize() const { return Size; }
		btCollisionShape* GetBulletShape() override;
	};

	class __declspec(dllexport) Sphere : public PhysicShape {
	private:
		float Radius;
		btCollisionShape* bShape;
	public:
		Sphere(float radius);

		float GetRadius() const { return Radius; }
		void SetRadius(float radius);

		btCollisionShape* GetBulletShape() override;
	};

	class __declspec(dllexport) MeshShape : public PhysicShape {
	private:
		Ref<Mesh> m_Mesh;
		btCollisionShape* bShape;
	public:
		MeshShape(Ref<Mesh> mesh);

		void SetMesh(Mesh* mesh);
		Mesh* GetMesh();

		btCollisionShape* GetBulletShape() override;
	};
}
