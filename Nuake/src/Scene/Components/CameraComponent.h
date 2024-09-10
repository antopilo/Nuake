#pragma once

#include "src/Core/Object/Object.h"

#include "TransformComponent.h"
#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"
#include "src/Rendering/Camera.h"

namespace Nuake
{
	
	class CameraComponent 
	{
		NUAKECOMPONENT(CameraComponent, "Camera")
		
	public:
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