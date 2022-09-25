#pragma once
#include <glm/ext/vector_float3.hpp>
#include "Rigibody.h"
#include "src/Core/Timestep.h"
#include "src/Core/Core.h"
#include <map>
#include "RaycastResult.h"

#include <src/Core/Physics/GhostObject.h>
#include "CharacterController.h"

namespace Nuake
{
	namespace Physics {
		class DynamicWorld {
		private:
		public:
			DynamicWorld();

			void DrawDebug();

			void SetGravity(glm::vec3 g);
			void AddRigidbody(Ref<RigidBody> rb);

			void AddGhostbody(Ref<GhostObject> gb);

			void AddCharacterController(Ref < CharacterController> cc);

			RaycastResult Raycast(glm::vec3 from, glm::vec3 to);
			void StepSimulation(Timestep ts);
			void Clear();

		};
	}
}

