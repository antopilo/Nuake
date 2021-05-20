#pragma once
#include "TransformComponent.h"
#include "../Core/Core.h"
#include "../Resource/Serializable.h"
class CameraComponent {
public:
	Ref<Camera> CameraInstance;
	TransformComponent* transformComponent;

	CameraComponent();

	void DrawEditor();

	json Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_OBJECT(CameraInstance);
		END_SERIALIZE();
	}

	bool Deserialize(std::string str)
	{
		CameraInstance = CreateRef<Camera>();

		return CameraInstance->Deserialize(str);
	}
};