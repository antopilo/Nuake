#include "PhysicsShapes.h"

namespace Nuake
{
	namespace Physics
	{
		Box::Box()
		{
			Size = Vector3(1);
			m_Type = BOX;
		}

		Box::Box(Vector3 size)
		{
			Size = size;
			m_Type = BOX;
		}

		Box::Box(float x, float y, float z) 
		{
			Size = Vector3(x, y, z);
			m_Type = BOX;
		}

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
