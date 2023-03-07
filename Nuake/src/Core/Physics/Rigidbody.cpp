#include "PhysicsShapes.h"
#include "Rigibody.h"
#include "../Core.h"
#include <glm/trigonometric.hpp>
#include <src/Scene/Entities/Entity.h>

namespace Nuake
{
	namespace Physics
	{
		RigidBody::RigidBody()
		{
			
		}

		RigidBody::RigidBody(glm::vec3 position, Entity handle)
		{
			
		}

		RigidBody::RigidBody(float mass, glm::vec3 position, Ref<PhysicShape> shape, glm::vec3 initialVel)
		{
			
		}

		void RigidBody::SetShape(Ref<PhysicShape> shape)
		{
			
		}

		void RigidBody::UpdateTransform()
		{
		}

		void RigidBody::SetEntityID(Entity ent)
		{
			
		}
	}
}
