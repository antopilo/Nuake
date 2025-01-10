#pragma once

#include "Component.h"

#include "TransformComponent.h"
#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"
#include "src/Rendering/Camera.h"
#include "src/Resource/UUID.h"

namespace Nuake
{
	
	class CameraComponent : public Component
	{
		NUAKECOMPONENT(CameraComponent, "Camera")
	public:
		UUID ID;
		Ref<Camera> CameraInstance;
		TransformComponent* transformComponent;

		CameraComponent();

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_OBJECT(CameraInstance);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			CameraInstance = CreateRef<Camera>();

			return CameraInstance->Deserialize(j);
		}
	};
}