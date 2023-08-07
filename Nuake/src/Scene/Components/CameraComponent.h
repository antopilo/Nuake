#pragma once
#include "TransformComponent.h"
#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"
#include "src/Rendering/Camera.h"

namespace Nuake
{
	class CameraComponent 
	{
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

		bool Deserialize(const json& j)
		{
			CameraInstance = CreateRef<Camera>();

			return CameraInstance->Deserialize(j);
		}
	};
}