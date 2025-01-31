#pragma once
#include "Nuake/Resource/UUID.h"
#include "Nuake/Scene/Systems/System.h"
#include "Nuake/Core/Maths.h"


namespace Nuake
{
	class UIResource;

	class UISystem : public System
	{
	public:
		UISystem(Scene* scene);
		bool Init() override;

		void Update(Timestep ts) override;
		void EditorUpdate() override;
		void FixedUpdate(Timestep ts) override;

		void Draw() override {}

		void Exit() override;

	private:
		std::map<UUID, Ref<UIResource>> uis;
	};
}
