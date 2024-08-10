#pragma once
#include "src/Core/Core.h"
#include "src/Core/Timestep.h"

#include <glm/ext/vector_float3.hpp>
#include "Rigibody.h"
#include <map>
#include "RaycastResult.h"

#include <src/Physics/GhostObject.h>
#include "CharacterController.h"
#include "CollisionData.h"

#include "Jolt/Jolt.h"

#include <mutex>

namespace JPH
{
	class PhysicsSystem;
	class JobSystemThreadPool;
	class ContactListener;
	class BodyActivationListener;
	class BodyInterface;
	class Shape;
	class CharacterVirtual;

	template<class T>
	class Ref;
}

namespace Nuake
{
	class BPLayerInterfaceImpl;
	class MyContactListener;
	class MyBodyActivationListener;

	namespace Physics
	{
		struct CharacterGhostPair
		{
			Ref<JPH::CharacterVirtual> Character;
			uint32_t Ghost;
		};



		class DynamicWorld 
		{
		private:
			uint32_t _stepCount;

			Ref<JPH::PhysicsSystem> _JoltPhysicsSystem;
			JPH::JobSystemThreadPool* _JoltJobSystem;
			Ref<MyContactListener> _contactListener;
			Ref<MyBodyActivationListener> _bodyActivationListener;
			JPH::BodyInterface* _JoltBodyInterface;
			BPLayerInterfaceImpl* _JoltBroadphaseLayerInterface;

			std::vector<uint32_t> _registeredBodies;
			std::map<uint32_t, CharacterGhostPair> _registeredCharacters;

			std::mutex _CollisionCallbackMutex;
			std::vector<CollisionData> _CollisionCallbacks;
		public:
			DynamicWorld();

			void DrawDebug();

			void SetGravity(const Vector3& g);
			void AddRigidbody(Ref<RigidBody> rb);

			void AddGhostbody(Ref<GhostObject> gb);
			void AddCharacterController(Ref<CharacterController> cc);
			bool IsCharacterGrounded(const Entity& entity);
			void SetCharacterControllerPosition(const Entity& entity, const Vector3& position);

			void SetBodyPosition(const Entity& entity, const Vector3& position, const Quat& rotation);

			// This is going to be ugly. TODO: Find a better way that passing itself as a parameter
			void MoveAndSlideCharacterController(const Entity& entity, const Vector3& velocity);
			void AddForceToRigidBody(Entity& entity, const Vector3& force);

			std::vector<RaycastResult> Raycast(const Vector3& from, const Vector3& to);
			std::vector<ShapeCastResult> CastShape(const Vector3& from, const Vector3& to, const Ref<PhysicShape>& shape);
			void StepSimulation(Timestep ts);
			void Clear();

			void ClearCollisionData();

			void RegisterCollisionCallback(const CollisionData& data);
			const std::vector<CollisionData>& GetCollisionsData();
		private:
			JPH::Ref<JPH::Shape> GetJoltShape(const Ref<PhysicShape> shape);
			void SyncEntitiesTranforms();
			void SyncCharactersTransforms();

			JPH::Vec3 CreateJoltVec3(const Vector3& input) const
			{
				return JPH::Vec3(input.x, input.y, input.z);
			}
		};
	}
}
