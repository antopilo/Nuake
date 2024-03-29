#pragma once
#include <src/Scene/Systems/System.h>
#include <src/Core/Maths.h>

namespace Nuake
{
	class AudioSystem : public System
	{
	private:
		std::unordered_map<uint32_t, bool> m_StatusCache;

	public:
		AudioSystem(Scene* scene);
		bool Init() override;
		void Update(Timestep ts) override;
		void Draw() override {}
		void EditorUpdate() override;
		void FixedUpdate(Timestep ts) override;
		void Exit() override;
	};
}
