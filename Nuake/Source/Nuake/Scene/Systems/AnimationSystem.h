#pragma once
#include "Nuake/Scene/Systems/System.h"
#include "Nuake/Core/Maths.h"

#include "Nuake/Resource/SkeletonNode.h"
#include "Nuake/Resource/SkeletalAnimation.h"

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
		void UpdateBonePositionTraversal(SkeletonNode& bone, Ref<SkeletalAnimation> animation, Ref<SkeletalAnimation> prevAnimation, float time, float prevTime, bool isPlaying, float blendTime);
	};
}
