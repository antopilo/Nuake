#include "PhysicsShapes.h"
#include "Rigibody.h"
#include "Nuake/Core/Core.h"
#include "Nuake/Physics/PhysicsManager.h"

#include <glm/trigonometric.hpp>
#include "Nuake/Scene/Entities/Entity.h"

namespace Nuake
{
	namespace Physics
	{
		RigidBody::RigidBody()
		{
			
		}

		RigidBody::RigidBody(Vector3 position, Entity handle) : _position(position)
		{
			
		}

		RigidBody::RigidBody(float mass, Vector3 position, Quat rotation, Matrix4 transform, Ref<PhysicShape> shape, Entity entity, Vector3 initialVel, bool forceKinematic) :
			_position(position),
			_collisionShape(shape),
			_mass(mass),
			_entity(entity),
			_rotation(rotation),
			_ForceKinematic(forceKinematic)
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
			_entity = ent;
		}

		void RigidBody::AddForce(const Vector3& force)
		{
			PhysicsManager::Get().GetWorld()->AddForceToRigidBody(_entity, force);
		}
	}
}
