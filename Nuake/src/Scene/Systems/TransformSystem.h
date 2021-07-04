#pragma once
#include "System.h"

class Scene;
class TransformSystem : public System {
public:
	TransformSystem(Scene* scene);
	bool Init() override;
	void Update(Timestep ts) override;
	void Draw() override {}
	void FixedUpdate(Timestep ts) override;
	void Exit() override;

	void UpdateTransform();
};