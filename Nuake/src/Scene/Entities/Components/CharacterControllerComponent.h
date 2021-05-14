#pragma once

#include "../Core/Physics/CharacterController.h"
class CharacterControllerComponent
{
public:
	Ref < Physics::CharacterController>  CharacterController;

	float Height = 1.0f;
	float Radius = 0.2f;
	float Mass = 25.0f;

	CharacterControllerComponent() 
	{
		
	}

	void SyncWithTransform(TransformComponent& tc)
	{
		btVector3 pos = CharacterController->m_motionTransform.getOrigin();
		glm::vec3 finalPos = glm::vec3(pos.x(), pos.y(), pos.z());

		std::string msg = "moved to: " + std::to_string(finalPos.x) + " " + std::to_string(finalPos.y) + "\n";
		printf(msg.c_str());
		tc.Translation = finalPos;
	}
};