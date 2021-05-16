#pragma once
#include <glm\ext\matrix_float4x4.hpp>
#include "BaseComponent.h";
#include "../Resource/Serializable.h"
class TransformComponent {
public:
	glm::vec3 Translation;
	glm::vec3 Rotation;
	glm::vec3 Scale;

	TransformComponent();
	glm::mat4 GetTransform();

	void DrawEditor();

	json Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL_LBL("Type", "TransformComponent");
		SERIALIZE_VEC3(Translation);
		SERIALIZE_VEC3(Rotation);
		SERIALIZE_VEC3(Scale);
		END_SERIALIZE();
	}
};