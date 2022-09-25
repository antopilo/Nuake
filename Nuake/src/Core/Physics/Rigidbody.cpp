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
			Ref<Box> shape = CreateRef<Box>();
			m_CollisionShape = shape;

			

			m_Mass = 0.0f;

			//rigidbody is dynamic if and only if mass is non zero, otherwise static
			m_IsDynamic = (m_Mass != 0.0f);

			

			m_InitialVel = glm::vec3(0, 0, 0);

			
		}

		RigidBody::RigidBody(float mass, glm::vec3 position, Ref<PhysicShape> shape, glm::vec3 initialVel)
		{
			m_CollisionShape = shape;

			m_Mass = mass;

			//rigidbody is dynamic if and only if mass is non zero, otherwise static
			m_IsDynamic = (m_Mass != 0.0f);

		}

		void RigidBody::SetShape(Ref<PhysicShape> shape)
		{
			m_CollisionShape = shape;
		}

		void RigidBody::UpdateTransform()
		{
		}

		glm::vec3 RigidBody::GetRotation() const {
			return { 0, 0, 0 };
		}

		void RigidBody::SetEntityID(Entity ent)
		{
			
		}

		void RigidBody::SetKinematic(bool value)
		{
			
		}

		glm::vec3 RigidBody::GetPosition() const {
			return { 0, 0, 0 };
		}

		void RigidBody::SetMass(float m) {  }
		void RigidBody::MoveAndSlide(glm::vec3 velocity)
		{

		}
	}
}
