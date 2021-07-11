#include "PhysicsShapes.h"
#include "btBulletDynamicsCommon.h"

namespace Nuake
{
	namespace Physics
	{
		Box::Box()
		{
			Size = glm::vec3(1);
			bShape = new btBoxShape(btVector3(Size.x, Size.y, Size.z));
			m_Type = BOX;
		}
		// Sphere
		Box::Box(glm::vec3 size) {
			Size = size;
			bShape = new btBoxShape(btVector3(size.x, size.y, size.z));
			m_Type = BOX;
		}

		Box::Box(float x, float y, float z) {
			Size = glm::vec3(x, y, z);
			bShape = new btBoxShape(btVector3(x, y, z));
			m_Type = BOX;
		}

		btCollisionShape* Box::GetBulletShape()
		{
			return bShape;
		}

		// Sphere
		Sphere::Sphere(float radius) {
			Radius = radius;
			bShape = new btSphereShape(Radius);

			m_Type = SPHERE;
		}

		void Sphere::SetRadius(float radius) {
			((btSphereShape*)bShape)->setUnscaledRadius(radius);
			Radius = radius;
		}

		btCollisionShape* Sphere::GetBulletShape()
		{
			return bShape;
		}

		MeshShape::MeshShape(Ref<Mesh> mesh)
		{
			m_Mesh = mesh;
			btConvexHullShape* trimesh = new btConvexHullShape();

			for (Vertex i : mesh->m_Vertices)
			{
				trimesh->addPoint(btVector3(i.position.x, i.position.y, i.position.z));
			}
			bShape = trimesh;
		}

		btCollisionShape* MeshShape::GetBulletShape()
		{
			return bShape;
		}
	}
}
