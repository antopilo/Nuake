#pragma once
#include <glm/ext/vector_float3.hpp>
#include "Rigibody.h"
#include "src/Core/Timestep.h"
#include "src/Core/Core.h"
#include <map>
#include "RaycastResult.h"

#include <src/Core/Physics/GhostObject.h>
#include "CharacterController.h"

#include <Jolt/Jolt.h>
#include "src/Core/Core.h"
namespace JPH
{
	class PhysicsSystem;
	class JobSystemThreadPool;
	class ContactListener;
	class BodyActivationListener;
	class BodyInterface;
	class Shape;

	template<class T>
	class Ref;
}


namespace Nuake
{
	class BPLayerInterfaceImpl;
	class MyContactListener;
	class MyBodyActivationListener;

	namespace Physics {
		class DynamicWorld 
		{
		private:
			uint32_t _stepCount;

			Ref<JPH::PhysicsSystem> _JoltPhysicsSystem;
			JPH::JobSystemThreadPool* _JoltJobSystem;
			Scope<MyContactListener> _contactListener;
			Scope<MyBodyActivationListener> _bodyActivationListener;
			JPH::BodyInterface* _JoltBodyInterface;
			BPLayerInterfaceImpl* _JoltBroadphaseLayerInterface;

			std::vector<uint32_t> _registeredBodies;
			std::vector<uint32_t> _registeredCharacters;
		public:
			DynamicWorld();

			void DrawDebug();

			void SetGravity(glm::vec3 g);
			void AddRigidbody(Ref<RigidBody> rb);

			void AddGhostbody(Ref<GhostObject> gb);
			void AddCharacterController(Ref<CharacterController> cc);

			RaycastResult Raycast(glm::vec3 from, glm::vec3 to);
			void StepSimulation(Timestep ts);
			void Clear();

		private:
			JPH::Ref<JPH::Shape> GetJoltShape(const Ref<PhysicShape> shape);
		};
	}
}

