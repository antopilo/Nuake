#pragma once

#include "Component.h"

#include "TransformComponent.h"
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/Serializable.h"
#include "Nuake/Rendering/Camera.h"
#include "Nuake/Resource/UUID.h"

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