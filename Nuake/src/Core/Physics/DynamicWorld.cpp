#include "DynamicWorld.h"
#include "Rigibody.h"
#include "../Core/Core.h"
#include <src/Vendors/glm/ext/quaternion_common.hpp>
#include <src/Core/Logger.h>

#include <Jolt/Jolt.h>


namespace Nuake
{
	namespace Physics
	{
		DynamicWorld::DynamicWorld() 
		{
			///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
			
			SetGravity(Vector3(0, -10000, 0));
		}


		void DynamicWorld::DrawDebug()
		{
		}


		void DynamicWorld::SetGravity(glm::vec3 g)
		{
		}


		void DynamicWorld::AddRigidbody(Ref<RigidBody> rb)
		{
			
		}

		void DynamicWorld::AddGhostbody(Ref<GhostObject> gb)
		{
		}

		void DynamicWorld::AddCharacterController(Ref<CharacterController> cc)
		{

			// Specify filters manually, otherwise ghost doesn't collide with statics for some reason
		}


		RaycastResult DynamicWorld::Raycast(glm::vec3 from, glm::vec3 to)
		{
			Vector3 localNorm = glm::vec3(0,0,0);

			//Logger::Log("normal: x:" + std::to_string(localNorm.x) + " y:" + std::to_string(localNorm.y )+ "z: " + std::to_string(localNorm.z));
			
			// Map bullet result to dto.
			RaycastResult result{
				glm::vec3(0,0,0),
				glm::vec3(0,0,0),
				localNorm
			};

			return result;
		}


		void DynamicWorld::StepSimulation(Timestep ts)
		{

		}


		void DynamicWorld::Clear()
		{
			
		}
	}

}
