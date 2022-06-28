#include "PhysicsShapes.h"
#include "btBulletDynamicsCommon.h"
#include "src/Rendering/Vertex.h"

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
			btTriangleMesh* trimesh = new btTriangleMesh();

			auto& indices = mesh->GetIndices();
			auto& vertices = mesh->GetVertices();
			for (unsigned int i = 0; i < std::size(indices); i += 3)
			{
				Vector3 tri1 = vertices[indices[i]].position;
				Vector3 tri2 = vertices[indices[i + 1]].position;
				Vector3 tri3 = vertices[indices[i + 2]].position;
			
				btVector3 btri1 = btVector3(tri1.x, tri1.y, tri1.z);
				btVector3 btri2 = btVector3(tri2.x, tri2.y, tri2.z);
				btVector3 btri3 = btVector3(tri3.x, tri3.y, tri3.z);
			
				trimesh->addTriangle(btri1, btri2, btri3, false);
			}
		
			bShape = new btBvhTriangleMeshShape(trimesh, false, true);;
		}

		btCollisionShape* MeshShape::GetBulletShape()
		{
			return bShape;
		}
	}
}
