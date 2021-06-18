#pragma once
#include "System.h"

class Scene;
class ScriptingSystem : public System {
public:
	ScriptingSystem(Scene* scene);
	void Init() override;
	void Update(Timestep ts) override;
	void Draw() override {}
	void FixedUpdate(Timestep ts) override;
	void Exit() override;
};