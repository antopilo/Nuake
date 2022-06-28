#pragma once
#include "PhysicsShapes.h"
#include "../Core/Core.h"
#include <glm/ext/vector_float3.hpp>
class btTransform;
class btRigidBody;
class btVector3;

namespace Nuake
{
	class Entity;
	namespace Physics {
		class RigidBody {
		private:
			bool m_IsDynamic = false;
			bool m_IsKinematic = false;
			glm::vec3 m_InitialVel;

			btRigidBody* m_Rigidbody;

			Ref<PhysicShape> m_CollisionShape;
		public:
			btTransform* m_Transform;
			float m_Mass;

			RigidBody();
			RigidBody(glm::vec3 position, Entity handle);
			RigidBody(float mass, glm::vec3 position, Ref<PhysicShape> shape, glm::vec3 initialVel = glm::vec3(0, 0, 0));

			btRigidBody* GetBulletRigidbody() const { return m_Rigidbody; }

			void UpdateTransform(btTransform t);
			glm::vec3 GetPosition() const;
			glm::vec3 GetRotation() const;

			void SetEntityID(Entity ent);

			void SetKinematic(bool value);
			bool IsKinematic() const { return m_IsKinematic; }

			bool HasShape() { return m_CollisionShape != nullptr; }
			void SetShape(Ref<PhysicShape> shape);
			Ref<PhysicShape> GetShape() const { return m_CollisionShape; }

			float GetMass() const { return m_Mass; }
			void SetMass(float m);

			void MoveAndSlide(glm::vec3 velocity);
		};
	}

}
