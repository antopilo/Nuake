#pragma once
#include "PhysicsShapes.h"
#include "../Core/Core.h"
#include <glm/ext/vector_float3.hpp>


namespace Nuake
{
	class Entity;
	namespace Physics {
		class RigidBody {
		private:
			bool m_IsDynamic = false;
			bool m_IsKinematic = false;
			Vector3 m_InitialVel;

			Ref<PhysicShape> m_CollisionShape;
		public:
			float m_Mass;

			RigidBody();
			RigidBody(glm::vec3 position, Entity handle);
			RigidBody(float mass, glm::vec3 position, Ref<PhysicShape> shape, glm::vec3 initialVel = glm::vec3(0, 0, 0));

			void UpdateTransform();
			Vector3 GetPosition() const;
			Vector3 GetRotation() const;

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
