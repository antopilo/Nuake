#pragma once
#include <glm/ext/vector_float3.hpp>
#include "Rigibody.h"
#include "src/Core/Timestep.h"
#include "src/Core/Core.h"
#include <map>
#include "RaycastResult.h"

#include <src/Core/Physics/GhostObject.h>
#include "CharacterController.h"

namespace JPH
{
	class PhysicsSystem;
	class JobSystemThreadPool;
	class ContactListener;
	class BodyActivationListener;
}

namespace Nuake
{
	class MyContactListener;
	class MyBodyActivationListener;

	namespace Physics {
		class DynamicWorld {
		private:
			uint32_t _stepCount;
			Ref<JPH::PhysicsSystem> _JoltPhysicsSystem;
			JPH::JobSystemThreadPool* _JoltJobSystem;
			Scope<MyContactListener> _contactListener;
			Scope<MyBodyActivationListener> _bodyActivationListener;

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
		};
	}
}

