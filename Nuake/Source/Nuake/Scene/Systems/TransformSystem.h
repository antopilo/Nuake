#pragma once
#include "System.h"

namespace Nuake {

	class Scene;
	class Entity;

	class TransformSystem : public System 
	{
	public:
		TransformSystem(Scene* scene);
		bool Init() override;
		void Update(Timestep ts) override;
		void Draw() override {}
		void FixedUpdate(Timestep ts) override;
		void Exit() override;

	private:
		void UpdateTransform();
		void UpdateDirtyFlagRecursive(Entity& entity);
		void CalculateGlobalTransform(Entity& entity);
	};
}
