#pragma once
#include <src/Scene/Systems/System.h>
#include <src/Core/Maths.h>

#include "src/Resource/SkeletonNode.h"
#include "src/Resource/SkeletalAnimation.h"

namespace Nuake
{
	class AnimationSystem : public System
	{
	public:
		AnimationSystem(Scene* scene);
		bool Init() override;
		void Update(Timestep ts) override;
		void Draw() override {}
		void EditorUpdate() override;
		void FixedUpdate(Timestep ts) override;
		void Exit() override;

	private:
		void UpdateBonePositionTraversal(SkeletonNode& bone, Ref<SkeletalAnimation> animation, float time, bool isPlaying);
	};
}
