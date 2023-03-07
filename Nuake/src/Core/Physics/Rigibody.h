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
			Ref<PhysicShape> m_CollisionShape;
		public:
			float m_Mass;

			RigidBody();
			RigidBody(glm::vec3 position, Entity handle);
			RigidBody(float mass, glm::vec3 position, Ref<PhysicShape> shape, glm::vec3 initialVel = glm::vec3(0, 0, 0));

			void UpdateTransform();

			void SetEntityID(Entity ent);

			bool HasShape() { return m_CollisionShape != nullptr; }
			void SetShape(Ref<PhysicShape> shape);
			Ref<PhysicShape> GetShape() const { return m_CollisionShape; }
		};
	}
}
