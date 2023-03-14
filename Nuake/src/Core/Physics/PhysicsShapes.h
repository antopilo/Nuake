#pragma once
#include "src/Core/Maths.h"
#include "src/Rendering/Mesh/Mesh.h"

namespace Nuake
{
	namespace Physics 
	{
		enum RigidbodyShapes 
		{
			BOX, SPHERE, CAPSULE, MESH, CYLINDER
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

		class Capsule : public PhysicShape
		{
		private:
			float Radius;
			float Height;
		public:
			Capsule(float radius, float height) :
				Radius(radius),
				Height(height)
			{
				m_Type = CAPSULE;
			}

			float GetRadius() const { return Radius; }
			void SetRadius(float radius) { Radius = radius; }
			float GetHeight() const { return Height; }
			void SetHeight(float height) { Height = height; }
		};

		class Cylinder : public PhysicShape
		{
		private:
			float Radius;
			float Height;
		public:
			Cylinder(float radius, float height) : Radius(radius), Height(height)
			{
				m_Type = CYLINDER;
			}

			float GetRadius() const { return Radius; }
			void SetRadius(float radius) { Radius = radius; }
			float GetHeight() const { return Height; }
			void SetHeight(float height) { Height = height; }
		};

		class MeshShape : public PhysicShape 
		{
		private:
			Ref<Mesh> m_Mesh;
		public:
			MeshShape(Ref<Mesh> mesh) : m_Mesh(mesh)
			{
				m_Type = MESH;
			}

			void SetMesh(Ref<Mesh> mesh) { m_Mesh = mesh; }
			Ref<Mesh> GetMesh() { return m_Mesh; }
		};
	}

}
