#pragma once
#include "../Core/Maths.h"
#include "../Resource/Serializable.h"

class TransformComponent {
public:
	Vector3 GlobalTranslation;
	Vector3 Translation;
	Vector3 Rotation; // TODO: Should use quaternions.
	Vector3 Scale;

	TransformComponent();

	Matrix4 GetTransform();

	json Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL_LBL("Type", "TransformComponent");
		SERIALIZE_VEC3(Translation);
		SERIALIZE_VEC3(Rotation);
		SERIALIZE_VEC3(Scale);
		END_SERIALIZE();
	}

	bool Deserialize(std::string str)
	{
		BEGIN_DESERIALIZE();
		this->Translation = Vector3(j["Translation"]["x"], j["Translation"]["y"], j["Translation"]["z"]);
		this->Rotation = Vector3(j["Rotation"]["x"], j["Rotation"]["y"], j["Rotation"]["z"]);
		this->Scale = Vector3(j["Scale"]["x"], j["Scale"]["y"], j["Scale"]["z"]);
		return true;
	}
};