#pragma once
#include <src/Scene/Systems/System.h>

class PhysicsSystem : public System {
public:
	PhysicsSystem(Scene* scene);
	void Init() override;
	void Update(Timestep ts) override;
	void Draw() override {}
	void FixedUpdate(Timestep ts) override;
	void Exit() override;
};