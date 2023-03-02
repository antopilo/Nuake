#pragma once
#include "src/Core/Maths.h"
#include "src/Rendering/Mesh/Mesh.h"

namespace Nuake
{
	namespace Physics 
	{
		enum RigidbodyShapes 
		{
			BOX, SPHERE, CAPSULE, MESH
		};

		class PhysicShape 
		{
		protected:
			RigidbodyShapes m_Type;
		public:
			RigidbodyShapes GetType() const { return m_Type; }

		};

		class Box : public PhysicShape 
		{
		private:
			glm::vec3 Size;
		public:
			Box();
			Box(glm::vec3 size);
			Box(float x, float y, float z);

			glm::vec3 GetSize() const { return Size; }
		};

		class Sphere : public PhysicShape 
		{
		private:
			float Radius;
		public:
			Sphere(float radius);

			float GetRadius() const { return Radius; }
			void SetRadius(float radius);

		};

		class MeshShape : public PhysicShape 
		{
		private:
			Ref<Mesh> m_Mesh;
		public:
			MeshShape(Ref<Mesh> mesh);

			void SetMesh(Mesh* mesh);
			Mesh* GetMesh();

		};
	}

}
