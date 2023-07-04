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
		Box::Box(glm::vec3 size) 
		{
			Size = size;
			m_Type = BOX;
		}

		Box::Box(float x, float y, float z) 
		{
			Size = glm::vec3(x, y, z);
			m_Type = BOX;
		}

		// Sphere
		Sphere::Sphere(float radius) 
		{
			Radius = radius;
			m_Type = SPHERE;
		}

		void Sphere::SetRadius(float radius) 
		{
			Radius = radius;
		}
	}
}
