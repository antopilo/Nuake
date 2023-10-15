#pragma once
#include "src/Core/Maths.h"
#include "src/Rendering/Mesh/Mesh.h"

namespace Nuake
{
	namespace Physics 
	{
		enum RigidbodyShapes 
		{
			BOX, SPHERE, CAPSULE, MESH, CYLINDER, CONVEX_HULL
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
			Vector3 Size;
		public:
			Box();
			Box(Vector3 size);
			Box(float x, float y, float z);

			Vector3 GetSize() const { return Size; }
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
			Capsule() = default;
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

		class ConvexHullShape : public PhysicShape
		{
		private:
			std::vector<Vector3> _points;

		public:
			ConvexHullShape(const std::vector<Vector3>& points) : _points(points)
			{
				m_Type = CONVEX_HULL;
			}

			std::vector<Vector3> GetPoints() const { return _points; }
			void SetPoints(const std::vector<Vector3>& points)
			{
				_points = points;
			}
		};
	}
}
