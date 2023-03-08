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
			Ref<PhysicShape> _collisionShape;
			Vector3 _position;
		public:
			float _mass;

			RigidBody();
			RigidBody(glm::vec3 position, Entity handle);
			RigidBody(float mass, glm::vec3 position, Ref<PhysicShape> shape, glm::vec3 initialVel = glm::vec3(0, 0, 0));

			void UpdateTransform();

			void SetEntityID(Entity ent);
			Vector3 GetPosition() const { return _position; }
			bool HasShape() { return _collisionShape != nullptr; }
			void SetShape(Ref<PhysicShape> shape);
			Ref<PhysicShape> GetShape() const { return _collisionShape; }
		};
	}
}
