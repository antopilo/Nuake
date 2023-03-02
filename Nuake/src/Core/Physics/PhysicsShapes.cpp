#include "PhysicsShapes.h"
#include "src/Rendering/Vertex.h"

namespace Nuake
{
	namespace Physics
	{
		Box::Box()
		{
			Size = glm::vec3(1);
			m_Type = BOX;
		}
		// Sphere
		Box::Box(glm::vec3 size) {
			Size = size;
			m_Type = BOX;
		}

		Box::Box(float x, float y, float z) {
			Size = glm::vec3(x, y, z);
			m_Type = BOX;
		}



		// Sphere
		Sphere::Sphere(float radius) {
			Radius = radius;

			m_Type = SPHERE;
		}

		void Sphere::SetRadius(float radius) {
			Radius = radius;
		}

		MeshShape::MeshShape(Ref<Mesh> mesh)
		{
			m_Mesh = mesh;

			auto& indices = mesh->GetIndices();
			auto& vertices = mesh->GetVertices();
			for (unsigned int i = 0; i < std::size(indices); i += 3)
			{
				Vector3 tri1 = vertices[indices[i]].position;
				Vector3 tri2 = vertices[indices[i + 1]].position;
				Vector3 tri3 = vertices[indices[i + 2]].position;
			
			
			}
		
		}

	}
}
