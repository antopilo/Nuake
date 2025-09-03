#pragma once
#include "Nuake/Scene/Systems/System.h"

namespace Nuake
{
	class PhysicsSystem : public System 
	{
	public:
		PhysicsSystem(Scene* scene);
		bool Init() override;
		void Update(Timestep ts) override;
		void Draw() override {}
		void FixedUpdate(Timestep ts) override;
		void Exit() override;

		void InitializeNewBodies();
	private:
		void InitializeShapes();
		void InitializeQuakeMap();
		void InitializeRigidbodies();
		void InitializeCharacterControllers();

		void ApplyForces();
	};
}
